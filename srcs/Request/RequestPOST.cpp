#include "RequestPOST.hpp"

#include <unistd.h>

#include "Server.hpp"
#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

RequestPOST::RequestPOST(RequestContext_t &context) : ARequest(context) {
	SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
	this->_contentTotalLength = 0;
}

RequestPOST::RequestPOST(const RequestPOST &other) : ARequest(other) { *this = other; }

RequestPOST::~RequestPOST(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

RequestPOST &RequestPOST::operator=(const RequestPOST &other) {
	(void)other;
	return (*this);
}

/* ************************************************************************** */

void RequestPOST::_openCGI(void) {
	if (0 != this->_cgiPath->access(X_OK)) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, this->_context.cgiSockets)) {
		std::cerr << "Error: socketpair(): " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}

	this->_context.option = EPOLLIN | EPOLLOUT;

	this->_context.pid = fork();
	if (-1 == this->_context.pid) {
		std::cerr << "Error: fork(): " << strerror(errno) << std::endl;
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
	}
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
	// Check headers
	if (REQ_CONTINUE != this->_checkHeaders()) {
		SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
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

	if (0 != upload.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
	} else if (0 != upload.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	} else if ((0 == this->_path.access(F_OK) && 0 == this->_path.stat() && this->_path.isDir()) ||
	           !upload.isDir()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
	} else if (0 != upload.access(W_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
	} else if (this->_context.ruleBlock->hasClientBodyTempPath() &&
				upload != temp && (0 != temp.stat() || !temp.isDir() || 0 != temp.access(W_OK) ||
				upload.deviceID() != temp.deviceID())) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	} else {
		this->_openFile();
	}
}

error_t RequestPOST::workIn(void) {
	if (this->_chunked) {
		return (this->_readChunked());
	} else {
		return (this->_readContent());
	}
	return (REQ_DONE);
}

error_t RequestPOST::workOut(void) {
	SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

error_t RequestPOST::CGIIn(void) { return (this->_readCGI()); }

error_t RequestPOST::CGIOut(void) {
	if (this->_chunked) {
		return (this->_readChunked());
	} else {
		return (this->_readContent());
	}
	return (REQ_ERROR);
}

ARequest *RequestPOST::clone(void) const { return (new RequestPOST(*this)); }

/* OTHERS *********************************************************************/

ARequest *createRequestPOST(RequestContext_t &context) { return (new RequestPOST(context)); }
