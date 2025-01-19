#include "RequestPUT.hpp"

#include <iostream>

/* CONSTRUCTORS ************************************************************* */

// RequestPUT::RequestPUT(void)
// {
// 	// std::cerr << "RequestPUT created" << std::endl;
// }

RequestPUT::RequestPUT(RequestContext_t &context) : ARequest(context) {
	std::cerr << "RequestPUT created" << std::endl;
}

RequestPUT::RequestPUT(const RequestPUT &other) : ARequest(other) {
	// std::cerr << "RequestPUT copy" << std::endl;
	*this = other;
}

RequestPUT::~RequestPUT(void) {
	// std::cerr << "RequestPUT destroyed" << std::endl;
	if (this->_file.is_open()) this->_file.close();
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestPUT &RequestPUT::operator=(const RequestPUT &other) {
	std::cerr << "RequestPUT assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

error_t RequestPUT::parse(void) {
	std::cerr << "RequestPUT parse" << std::endl;
	SET_REQ_READ_BODY_COMPLETE(this->_context.requestState);
	headers_t::const_iterator it = this->_context.headers.find(HEADER_CONTENT_LENGTH);
	if (it == this->_context.headers.end()) {
		it = this->_context.headers.find(HEADER_TRANSFER_ENCODING);
		if (it == this->_context.headers.end()) {
			this->_context.response.setStatusCode(STATUS_LENGTH_REQUIRED);
			SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
			return (REQ_DONE);
		}
		if (it->second != HEADER_TRANSFER_CHUNKE) {
			this->_context.response.setStatusCode(STATUS_UNSUPPORTED_MEDIA_TYPE);
			SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
			return (REQ_DONE);
		}
		this->_chunked = true;
	}
	if (!this->_chunked) {
		this->_contentLength = sToContentLength(it->second);
		if (this->_contentLength == CONTENT_LENGTH_INVALID) {
			this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
			SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
			return (REQ_DONE);
		}
		if (this->_contentLength == CONTENT_LENGTH_TOO_LARGE) {
			this->_context.response.setStatusCode(STATUS_PAYLOAD_TOO_LARGE);
			SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
			return (REQ_DONE);
		}
	}
	return (REQ_DONE);
}

error_t RequestPUT::processIn(void) {
	std::cerr << "RequestPUT processIn" << std::endl;
	// debug start
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setBody("Hello, World!");
	SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
	// debug end
	return (REQ_DONE);
}

error_t RequestPUT::processOut(void) {
	std::cerr << "RequestPUT processOut" << std::endl;
	SET_REQ_PROCESS_OUT_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

ARequest *RequestPUT::clone(void) const {
	std::cerr << "RequestPUT clone" << std::endl;
	return (new RequestPUT(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest *createRequestPUT(RequestContext_t &context) {
	std::cerr << "createRequestPUT" << std::endl;
	return (new RequestPUT(context));
}
