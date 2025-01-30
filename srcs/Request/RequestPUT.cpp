#include "RequestPUT.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>

#include <cstring>
#include <iostream>

#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

// RequestPUT::RequestPUT(void)
// {
// 	// std::cerr << "RequestPUT created" << std::endl;
// }

RequestPUT::RequestPUT(RequestContext_t &context)
    : ARequest(context) {
	// std::cerr << "RequestPUT created" << std::endl;
	SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);	// No workOut needed
	SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);	// TMP
	SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);	// TMP
	this->_contentTotalLength = 0;
}

RequestPUT::RequestPUT(const RequestPUT &other)
    : ARequest(other) {
	// std::cerr << "RequestPUT copy" << std::endl;
	*this = other;
}

RequestPUT::~RequestPUT(void) {
	// std::cerr << "RequestPUT destroyed" << std::endl;
	if (this->_file.is_open()) {
		this->_file.close();
	}
	if (0 == this->_tmpFilename.access(F_OK)) {
		std::remove(this->_tmpFilename.c_str());
	}
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestPUT &RequestPUT::operator=(const RequestPUT &other) {
	// std::cerr << "RequestPUT assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

error_t RequestPUT::_checkHeaders(void) {
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
		this->_contentLength = -1;	// -1: Need to read chunk size
	}
	return (REQ_CONTINUE);
}

/* ************************************************************************** */

void RequestPUT::processing(void) {
	std::cerr << "RequestPUT processing" << std::endl;
	// Check headers
	if (REQ_CONTINUE != this->_checkHeaders()) {
		return;
	}
	std::cerr << "PUT Path:" << this->_path << std::endl;
	// Check path
	if (this->_path.isDirFormat()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		return;
	}
	if (!this->_context.ruleBlock->canUpload()) {
		this->_context.response.setStatusCode(STATUS_METHOD_NOT_ALLOWED);
		return;
	}

	uint32_t matchLength = this->_context.ruleBlock->path().string().size() - 1;

	this->_path = this->_context.ruleBlock->clientBodyUploadPath().string() + this->_context.target.substr(matchLength, std::string::npos);
	Path upload = this->_path.dir();
	Path temp = this->_context.ruleBlock->clientBodyTempPath();

	std::cerr << "DEBUG UPLOAD: " << upload.string() << "\n" << temp.string() << "\n" << this->path() << std::endl;

	if (0 != upload.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
	} else if (0 != upload.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	} else if (!upload.isDir()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
	} else if (0 != upload.access(W_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
	} else if ( upload != temp && (0 != temp.stat() || !temp.isDir() || 0 != temp.access(W_OK) || upload.deviceID() != temp.deviceID())) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	} else {
		this->_openFile();
	}
}

error_t RequestPUT::workIn(void) {
	// std::cerr << "RequestPUT workIn" << std::endl;
	if (this->_chunked) {
		return (this->_readChunked());
	} else {
		return (this->_readContent());
	}
	return (REQ_DONE);
}

ARequest *RequestPUT::clone(void) const {
	// std::cerr << "RequestPUT clone" << std::endl;
	return (new RequestPUT(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest *createRequestPUT(RequestContext_t &context) {
	// std::cerr << "createRequestPUT" << std::endl;
	return (new RequestPUT(context));
}
