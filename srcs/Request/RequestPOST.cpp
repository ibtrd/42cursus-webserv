#include "RequestPOST.hpp"

#include <unistd.h>


#include "Server.hpp"
#include "ft.hpp"
#include "webservHTML.hpp"
#include "CgiBuilder.hpp"

/* CONSTRUCTORS ************************************************************* */

RequestPOST::RequestPOST(RequestContext_t &context)
	: ARequest(context), _chunked(false), _contentLength(0) {
	// std::cerr << "RequestPOST created" << std::endl;
	SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);  // No workIn needed
	// SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
	// SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
	this->_contentTotalLength = 0;
}

RequestPOST::RequestPOST(const RequestPOST &other)
    : ARequest(other), _chunked(false), _contentLength(0) {
	// std::cerr << "RequestPOST copy" << std::endl;
	*this = other;
}

RequestPOST::~RequestPOST(void) {
	// std::cerr << "RequestPOST destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestPOST &RequestPOST::operator=(const RequestPOST &other) {
	// std::cerr << "RequestPOST assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

void RequestPOST::_openCGI(void) {
	std::cerr << "RequestPOST _openCGI" << std::endl;
	if (0 != this->_cgiPath->access(X_OK)) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, this->_context.cgiSockets)) {
		std::cerr << "Error: socketpair: " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}

	this->_context.option = EPOLLIN | EPOLLOUT;

	this->_context.pid = fork();
	if (-1 == this->_context.pid) {
		std::cerr << "Error: fork: " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}

	if (this->_context.pid == 0) {
		try {
			this->_executeCGI();
		} catch (...) {
			std::exit(1);
		}
	} else {
		close(this->_context.cgiSockets[CHILD_SOCKET]);
		SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		UNSET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
		std::cerr << "RequestPOST CGI: " << this->_cgiPath->string() << std::endl;
	}
}

void RequestPOST::_saveFile(void) {
	std::cerr << "RequestPOST _saveFile" << std::endl;

	shutdown(this->_context.cgiSockets[PARENT_SOCKET], SHUT_WR);
	this->_context.response.setStatusCode(STATUS_OK);

	struct epoll_event event;
	event.events  = EPOLLIN;
	event.data.fd = this->_context.cgiSockets[PARENT_SOCKET];
	if (-1 == epoll_ctl(Client::epollFd, EPOLL_CTL_MOD, this->_context.cgiSockets[PARENT_SOCKET], &event)) {
		throw std::runtime_error("epoll_ctl: " + std::string(strerror(errno)));
	}

	std::cerr << "RequestPOST shutdown" << std::endl;
}

error_t RequestPOST::_readContent(void) {
	std::cerr << "RequestPOST _readContent" << std::endl;
	if (this->_contentLength - static_cast<int32_t>(this->_context.buffer.size()) >= 0) {

		std::cerr << "J'ENVOIE AU CGI" << std::endl;

		ssize_t bytes = send(this->_context.cgiSockets[PARENT_SOCKET], this->_context.buffer.data(), this->_context.buffer.size(), MSG_NOSIGNAL);
		if (bytes == -1) {
			std::cerr << "Error: send: " << strerror(errno) << std::endl;
			// throw std::runtime_error("RequestPOST::_readContent: send: " + std::string(strerror(errno)));
			return (REQ_ERROR);
		}
		this->_contentLength -= this->_context.buffer.size();
		this->_context.buffer.clear();
		// std::cerr << ".";	// DEBUG
		std::cerr << "Remaining contentLength: " << this->_contentLength << std::endl;
	} else {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
		return (REQ_CONTINUE);
	}
	if (this->_contentLength == 0) {
		this->_saveFile();
		SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
		return (REQ_CONTINUE);
	}
	return (REQ_CONTINUE);
}
/*
AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEFFFFFFFFFFGGGGGGGGGGHHHHHHHHHHIIIIIIIIIIJJJJJJJJJJKKKKKKKKKKLLLLLLLLLMMMMMMMMMNNNNNNNNNOOOOOOOOOPPPPPPPPP
*/
error_t RequestPOST::_readChunked(void) {
	// std::cerr << "RequestPOST _readChunked" << std::endl;
	ssize_t bytes = 0;

	while (!this->_context.buffer.empty()) {
		// std::cerr << "RequestPOST begin buffer: |" << this->_context.buffer << "|" << std::endl;

		// Read end of chunk
		if (this->_contentLength == 0) {
			if (this->_context.buffer.size() >= 2) {
				size_t pos = this->_context.buffer.rfind("\r\n", 0);
				if (pos == std::string::npos) {
					this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
					SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
					return (REQ_CONTINUE);
				}
				this->_context.buffer.erase(0, pos + 2);
				this->_contentLength = -1;
			} else if (this->_context.buffer.size() == 1 && this->_context.buffer[0] != '\r') {
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
				return (REQ_CONTINUE);
			} else {
				return (REQ_CONTINUE);
			}
		}

		// Read chunk size
		if (-1 == this->_contentLength) {
			// std::cerr << "RequestPOST contentLength -1" << std::endl;
			size_t pos = this->_context.buffer.find("\r\n");
			if (pos == std::string::npos) {
				return (REQ_CONTINUE);
			}
			std::string line = this->_context.buffer.substr(0, pos);
			if (line.empty()) {  // refuse empty chunk
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
				return (REQ_CONTINUE);
			}
			// this->_context.buffer.erase(0, pos + 2);
			this->_contentLength = sToContentLength(line, true);
			if (this->_contentLength == CONTENT_LENGTH_INVALID) {
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
				return (REQ_CONTINUE);
			}
			// Read end of transfer
			if (this->_contentLength == 0) {
				if (0 == this->_context.buffer.compare(ARequest::_chunkTerminator[0])) {
					this->_saveFile();
				} else {
					for (size_t i = 1; i < CHUNK_TERMINATOR_SIZE; ++i) {
						if (0 == this->_context.buffer.compare(ARequest::_chunkTerminator[i])) {
							this->_contentLength = -1;
							return (REQ_CONTINUE);
						}
					}
					this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				}

				SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
				return (REQ_CONTINUE);
			}
			this->_context.buffer.erase(0, pos + 2);
			this->_contentTotalLength += this->_contentLength;
			if (this->_contentLength == CONTENT_LENGTH_TOO_LARGE ||
			    this->_contentLength > this->_context.ruleBlock->getMaxBodySize() ||
			    this->_contentTotalLength > this->_context.ruleBlock->getMaxBodySize()) {
				this->_context.response.setStatusCode(STATUS_PAYLOAD_TOO_LARGE);
				SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
				return (REQ_CONTINUE);
			}
		}
		// std::cerr << "RequestPOST contentLength: " << this->_contentLength << std::endl;
		// std::cerr << "RequestPOST content buffer: |" << this->_context.buffer << "|" << std::endl;

		// Read chunk
		if (static_cast<int32_t>(this->_context.buffer.size()) > this->_contentLength) {

			std::cerr << "J'ENVOIE AU CGI" << std::endl;

			bytes = send(this->_context.cgiSockets[PARENT_SOCKET], this->_context.buffer.data(), this->_contentLength, MSG_NOSIGNAL);
			if (bytes == -1) {
				std::cerr << "Error: send: " << strerror(errno) << std::endl;
				// throw std::runtime_error("RequestPOST::_readContent: send: " + std::string(strerror(errno)));
				return (REQ_ERROR);
			}
			this->_context.buffer.erase(0, this->_contentLength);
			this->_contentLength = 0;
		} else if (static_cast<int32_t>(this->_context.buffer.size()) <= this->_contentLength) {

			std::cerr << "J'ENVOIE AU CGI" << std::endl;

			bytes = send(this->_context.cgiSockets[PARENT_SOCKET], this->_context.buffer.data(), this->_context.buffer.size(), MSG_NOSIGNAL);
			if (bytes == -1) {
				std::cerr << "Error: send: " << strerror(errno) << std::endl;
				// throw std::runtime_error("RequestPOST::_readContent: send: " + std::string(strerror(errno)));
				return (REQ_ERROR);
			}
			this->_contentLength -= this->_context.buffer.size();
			this->_context.buffer.clear();
		}
	}
	return (REQ_CONTINUE);
}

error_t RequestPOST::_executeCGI(void) {
	CgiBuilder builder(this);
	
	// std::cerr << builder;

	char **envp = builder.envp();
	char **argv = builder.argv();

	// std::cerr << "CGI-argv:\n";
	// for (uint32_t i = 0; argv[i]; ++i) {
	// 	std::cerr << argv[i] << "\n";
	// }
	// std::cerr << "CGI-envp:\n";
	// for (uint32_t i = 0; envp[i]; ++i) {
	// 	std::cerr << envp[i] << "\n";
	// }
	// std::cerr.flush();

	dup2(this->_context.cgiSockets[CHILD_SOCKET], STDOUT_FILENO);
	// dup2(this->_context.cgiSockets[CHILD_SOCKET], STDERR_FILENO);
	dup2(this->_context.cgiSockets[CHILD_SOCKET], STDIN_FILENO);
	// close(this->_context.cgiSockets[PARENT_SOCKET]);

	execve(this->_cgiPath->string().c_str(), argv, envp);

	std::cerr << "execve(): " << strerror(errno) << std::endl;

	CgiBuilder::destroy(envp);
	CgiBuilder::destroy(argv);
	std::exit(1);
}

error_t RequestPOST::_checkHeaders(void) {
	std::cerr << "CHECK HEADER" << std::endl;
	headers_t::const_iterator it = this->_context.headers.find(HEADER_CONTENT_LENGTH);
	if (it == this->_context.headers.end()) {
		it = this->_context.headers.find(HEADER_TRANSFER_ENCODING);
		if (it == this->_context.headers.end()) {
			this->_context.response.setStatusCode(STATUS_LENGTH_REQUIRED);
			return (REQ_DONE);
		}
		if (it->second != HEADER_TRANSFER_CHUNKE) {
			this->_context.response.setStatusCode(STATUS_UNSUPPORTED_MEDIA_TYPE);
			return (REQ_DONE);
		}
		this->_chunked = true;
	}
	if (!this->_chunked) {
		this->_contentLength = sToContentLength(it->second, false);
		if (this->_contentLength == CONTENT_LENGTH_INVALID) {
			this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
			return (REQ_DONE);
		}
		if (this->_contentLength == CONTENT_LENGTH_TOO_LARGE ||
		    this->_contentLength > this->_context.ruleBlock->getMaxBodySize()) {
			this->_context.response.setStatusCode(STATUS_PAYLOAD_TOO_LARGE);
			return (REQ_DONE);
		}
		if (this->_contentLength == 0) {
			this->_context.response.setStatusCode(STATUS_NO_CONTENT);
			return (REQ_DONE);
		}
		std::cerr << "QQQQQQQQQQQQQQQQQ RequestPOST contentLength: " << this->_contentLength << std::endl;
	} else {
		this->_contentLength = -1;
	}
	return (REQ_CONTINUE);
}

error_t RequestPOST::_validateLocalFile(void) {
	// if (0 != this->_path.stat()) {
	// 	this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	// 	return (REQ_DONE);
	// }
	// if (0 != this->_path.access(R_OK)) {
	// 	this->_context.response.setStatusCode(STATUS_FORBIDDEN);
	// 	return (REQ_DONE);
	// }
	// if (this->_path.isFile()) {
		this->_openCGI();
		return (REQ_DONE);
	// }
	// return (REQ_CONTINUE);
}

/* ************************************************************************** */

void RequestPOST::processing(void) {
	// std::cerr << "RequestPOST parse" << std::endl;
	// Check headers
	if (REQ_CONTINUE != this->_checkHeaders()) {
		return;
	}

	if (!this->_cgiPath) {
		this->_context.response.setStatusCode(STATUS_NOT_IMPLEMENTED);
		return;
	}
	// if (0 != this->_path.access(F_OK)) {
	// 	this->_context.response.setStatusCode(STATUS_NOT_FOUND);
	// 	return;
	// }
	if (REQ_CONTINUE != this->_validateLocalFile()) {
		return;
	}
	// this->_context.response.setStatusCode(STATUS_FORBIDDEN); //OG
	this->_context.response.setStatusCode(STATUS_NOT_FOUND);  // TESTER
}

error_t RequestPOST::workOut(void) {
	std::cerr << "RequestPOST workOut" << std::endl;
	SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

error_t RequestPOST::CGIIn(void) {
	// std::cerr << "RequestPOST CGIInt" << std::endl;
	return (this->_readCGI());
}

error_t RequestPOST::CGIOut(void) {
	// std::cerr << "RequestPOST CGIOut" << std::endl;

	if (this->_chunked) {
		return (this->_readChunked());
	} else {
		return (this->_readContent());
	}
	return (REQ_ERROR);
}

ARequest *RequestPOST::clone(void) const {
	// std::cerr << "RequestPOST clone" << std::endl;
	return (new RequestPOST(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest *createRequestPOST(RequestContext_t &context) {
	// std::cerr << "createRequestPOST" << std::endl;
	return (new RequestPOST(context));
}
