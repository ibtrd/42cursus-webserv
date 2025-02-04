#include "RequestPOST.hpp"

#include <unistd.h>

#include "CgiBuilder.hpp"
#include "Server.hpp"
#include "ft.hpp"
#include "webservHTML.hpp"

/* CONSTRUCTORS ************************************************************* */

RequestPOST::RequestPOST(RequestContext_t &context) : ARequest(context) {
	// std::cerr << "RequestPOST created" << std::endl;
	// SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);  // No workIn needed
	// SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
	// SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
	SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);  // No workOut needed
	this->_contentTotalLength = 0;
}

RequestPOST::RequestPOST(const RequestPOST &other) : ARequest(other) {
	// std::cerr << "RequestPOST copy" << std::endl;
	*this = other;
}

RequestPOST::~RequestPOST(void) {
	// std::cerr << "RequestPOST destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestPOST &RequestPOST::operator=(const RequestPOST &other) {
	// std::cerr << "RequestPOST assign" << std::endl;
	// int32_t _contentLength;

	// int32_t _contentTotalLength;
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

// void RequestPOST::_saveFile(void) {
// 	std::cerr << "RequestPOST _saveFile" << std::endl;

// 	shutdown(this->_context.cgiSockets[PARENT_SOCKET], SHUT_WR);
// 	this->_context.response.setStatusCode(STATUS_OK);

// 	struct epoll_event event;
// 	event.events  = EPOLLIN;
// 	event.data.fd = this->_context.cgiSockets[PARENT_SOCKET];
// 	if (-1 == epoll_ctl(Client::epollFd, EPOLL_CTL_MOD, this->_context.cgiSockets[PARENT_SOCKET],
// &event)) { 		throw std::runtime_error("epoll_ctl: " + std::string(strerror(errno)));
// 	}

// 	std::cerr << "RequestPOST shutdown" << std::endl;
// }

error_t RequestPOST::_executeCGI(void) {
	if (-1 == dup2(this->_context.cgiSockets[CHILD_SOCKET], STDOUT_FILENO) ||
	    -1 == dup2(this->_context.cgiSockets[CHILD_SOCKET], STDIN_FILENO)) {
		std::exit(1);
	}
	close(this->_context.cgiSockets[PARENT_SOCKET]);
	close(this->_context.cgiSockets[CHILD_SOCKET]);

	CgiBuilder builder(this);

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

	execve(this->_cgiPath->string().c_str(), argv, envp);
	std::cerr << "execve(): " << strerror(errno) << std::endl;

	CgiBuilder::destroy(envp);
	CgiBuilder::destroy(argv);
	std::exit(1);
}

error_t RequestPOST::_checkHeaders(void) {
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
	} else {
		this->_contentLength = -1;  // -1: Need to read chunk size
	}
	return (REQ_CONTINUE);
}

/* ************************************************************************** */

void RequestPOST::processing(void) {
	std::cerr << "RequestPOST parse" << std::endl;
	// Check headers
	if (REQ_CONTINUE != this->_checkHeaders()) {
		SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);  // To change
		SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
		return;
	}

	// CGI
	if (this->_cgiPath) {
		SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
		this->_openCGI();
		return;
	}
	SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);

	// Upload
	if (this->_path.isDirFormat()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		return;
	}
	if (!this->_context.ruleBlock->canUpload()) {
		this->_context.response.setStatusCode(STATUS_METHOD_NOT_ALLOWED);
		return;
	}

	uint32_t matchLength = this->_context.ruleBlock->path().string().size() - 1;

	this->_path = this->_context.ruleBlock->clientBodyUploadPath().string() +
	              this->_context.target.substr(matchLength, std::string::npos);
	Path upload = this->_path.dir();
	Path temp   = this->_context.ruleBlock->clientBodyTempPath();

	std::cerr << "UPLOAD PARAMS:\n_path=" << this->_path << "\nupload=" << upload.string()
	          << "\ntemp=" << temp.string() << std::endl;

	if (0 != upload.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
	} else if (0 != upload.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	} else if ((0 == this->_path.access(F_OK) && 0 == this->_path.stat() && this->_path.isDir()) ||
	           !upload.isDir()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
	} else if (0 != upload.access(W_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
	} else if (upload != temp && (0 != temp.stat() || !temp.isDir() || 0 != temp.access(W_OK) ||
	                              upload.deviceID() != temp.deviceID())) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	} else {
		this->_openFile();
	}
}

error_t RequestPOST::workIn(void) {
	std::cerr << "RequestPOST workIn" << std::endl;
	if (this->_chunked) {
		return (this->_readChunked());
	} else {
		return (this->_readContent());
	}
	return (REQ_DONE);
}

error_t RequestPOST::workOut(void) {
	std::cerr << "RequestPOST workOut" << std::endl;
	SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

error_t RequestPOST::CGIIn(void) {
	std::cerr << "RequestPOST CGIIn" << std::endl;
	return (this->_readCGI());
}

error_t RequestPOST::CGIOut(void) {
	std::cerr << "RequestPOST CGIOut" << std::endl;

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
