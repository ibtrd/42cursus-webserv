#include <unistd.h>

#include "Client.hpp"
#include "ft.hpp"

error_t ARequest::_generateFilename(void) {
	std::string basename =
	    this->_context.ruleBlock->clientBodyTempPath().string() + this->_path.notdir();
	std::string tmp;
	int32_t     i = 0;

	std::cerr << "tmp: " << this->_context.ruleBlock->clientBodyTempPath() << std::endl;
	std::cerr << "basename: " << basename << std::endl;
	do {
		tmp = basename + '.' + ft::generateRandomString(8) + ".tmp";
	} while (0 == access(tmp.c_str(), F_OK) && ++i < 100);
	if (i == 100) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return (REQ_DONE);
	}
	this->_tmpFilename = tmp;
	std::cerr << "tmpFilename: " << this->_tmpFilename << std::endl;
	return (REQ_CONTINUE);
}

void ARequest::_openFile(void) {
	if (REQ_CONTINUE != this->_generateFilename()) {
		return;
	}
	this->_file.open(this->_tmpFilename.c_str(),
	                 std::ios::out | std::ios::trunc | std::ios::binary);
	if (!this->_file.is_open()) {
		std::cerr << "open(): " << std::strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	}
}

void ARequest::_saveFile(void) {
	std::cerr << "ARequest _saveFile" << std::endl;

	if (this->_cgiPath) {
		shutdown(this->_context.cgiSockets[PARENT_SOCKET], SHUT_WR);
		this->_context.response.setStatusCode(STATUS_OK);

		struct epoll_event event;
		event.events  = EPOLLIN;
		event.data.fd = this->_context.cgiSockets[PARENT_SOCKET];
		if (-1 == epoll_ctl(Client::epollFd, EPOLL_CTL_MOD,
		                    this->_context.cgiSockets[PARENT_SOCKET], &event)) {
			throw std::runtime_error("epoll_ctl: " + std::string(strerror(errno)));
		}

		std::cerr << "RequestPOST shutdown" << std::endl;
		return;
	}
	this->_file.close();
	this->_context.response.setStatusCode(STATUS_CREATED);
	if (0 == this->_path.access(F_OK) && 0 != std::remove(this->_path.c_str())) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	}
	std::cerr << "RENAMEPARAMS:\nold" << this->_tmpFilename.c_str()
	          << "\nnew: " << this->_path.c_str() << std::endl;
	if (0 != std::rename(this->_tmpFilename.c_str(), this->_path.c_str())) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		std::cerr << "Error: rename(): " << std::strerror(errno) << std::endl;
	}
	std::remove(this->_tmpFilename.c_str());
}

error_t ARequest::_writeChunk(size_t size) {
	if (this->_cgiPath) {
		ssize_t bytes = send(this->_context.cgiSockets[PARENT_SOCKET], this->_context.buffer.data(),
		                     size, MSG_NOSIGNAL);
		if (bytes == -1) {
			std::cerr << "Error: send: " << strerror(errno) << std::endl;
			// throw std::runtime_error("RequestPOST::_readContent: send: " +
			// std::string(strerror(errno)));
			return (REQ_ERROR);
		}
		return (REQ_CONTINUE);
	}
	this->_file.write(this->_context.buffer.c_str(), this->_contentLength);
	return (REQ_CONTINUE);
}

error_t ARequest::_readContent(void) {
	if (this->_contentLength - static_cast<int32_t>(this->_context.buffer.size()) >= 0) {
		if (this->_writeChunk(this->_context.buffer.size()) == REQ_ERROR) {
			return (REQ_ERROR);
		}
		this->_contentLength -= this->_context.buffer.size();
		this->_context.buffer.clear();
	} else {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
		SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
		return (this->_cgiPath ? REQ_CONTINUE : REQ_DONE);
	}
	if (this->_contentLength == 0) {
		this->_saveFile();
		SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
		SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
		return (this->_cgiPath ? REQ_CONTINUE : REQ_DONE);
	}
	return (REQ_CONTINUE);
}

error_t ARequest::_readChunked(void) {
	// std::cerr << "RequestPUT _readChunked" << std::endl;
	while (!this->_context.buffer.empty()) {
		// std::cerr << "ARequest begin buffer: |" << this->_context.buffer << "|" << std::endl;

		// Read end of chunk
		if (this->_contentLength == 0) {
			if (this->_context.buffer.size() >= 2) {
				size_t pos = this->_context.buffer.rfind("\r\n", 0);
				if (pos == std::string::npos) {
					this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
					SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
					SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
					return (REQ_DONE);
				}
				this->_context.buffer.erase(0, pos + 2);
				this->_contentLength = -1;
			} else if (this->_context.buffer.size() == 1 && this->_context.buffer[0] != '\r') {
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
				SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
				return (this->_cgiPath ? REQ_CONTINUE : REQ_DONE);
			} else {
				return (REQ_CONTINUE);
			}
		}

		// Read chunk size
		if (-1 == this->_contentLength) {
			size_t pos = this->_context.buffer.find("\r\n");
			if (pos == std::string::npos) {
				return (REQ_CONTINUE);
			}
			std::string line = this->_context.buffer.substr(0, pos);
			if (line.empty()) {  // refuse empty chunk
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
				SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
				return (this->_cgiPath ? REQ_CONTINUE : REQ_DONE);
			}
			this->_contentLength = sToContentLength(line, true);
			if (this->_contentLength == CONTENT_LENGTH_INVALID) {
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
				SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
				return (this->_cgiPath ? REQ_CONTINUE : REQ_DONE);
			}
			// Read end of transfer
			if (this->_contentLength == 0) {
				if (0 == this->_context.buffer.compare(ARequest::_chunkTerminator[0])) {
					this->_context.buffer.clear();
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

				SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
				SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
				return (this->_cgiPath ? REQ_CONTINUE : REQ_DONE);
			}
			this->_context.buffer.erase(0, pos + 2);
			this->_contentTotalLength += this->_contentLength;
			if (this->_contentLength == CONTENT_LENGTH_TOO_LARGE ||
			    this->_contentLength > this->_context.ruleBlock->getMaxBodySize() ||
			    this->_contentTotalLength > this->_context.ruleBlock->getMaxBodySize()) {
				this->_context.response.setStatusCode(STATUS_PAYLOAD_TOO_LARGE);
				SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
				SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
				return (this->_cgiPath ? REQ_CONTINUE : REQ_DONE);
			}
		}

		// std::cerr << "ARequest chunk size: " << this->_contentLength << std::endl;
		// std::cerr << "ARequest inter buffer: |" << this->_context.buffer << "|" << std::endl;

		// Read chunk
		if (static_cast<int32_t>(this->_context.buffer.size()) > this->_contentLength) {
			if (this->_writeChunk(this->_contentLength) == REQ_ERROR) {
				return (REQ_ERROR);
			}
			this->_context.buffer.erase(0, this->_contentLength);
			this->_contentLength = 0;
		} else if (static_cast<int32_t>(this->_context.buffer.size()) <= this->_contentLength) {
			if (this->_writeChunk(this->_context.buffer.size()) == REQ_ERROR) {
				return (REQ_ERROR);
			}
			this->_contentLength -= this->_context.buffer.size();
			this->_context.buffer.clear();
		}
	}
	return (REQ_CONTINUE);
}
