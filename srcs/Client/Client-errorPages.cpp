#include <unistd.h>

#include "Server.hpp"
#include "ft.hpp"
#include "webservHTML.hpp"

error_t Client::_openErrorPage(void) {
	if (!this->_context.serverBlock) {
		return REQ_ERROR;
	}

	const Path *errorPathPtr =
	    this->_context.serverBlock->findErrorPage(this->_context.response.statusCode());
	if (!errorPathPtr) {
		return REQ_ERROR;
	}

	Path errorPath = *errorPathPtr;
	if (0 == errorPath.access(F_OK) && 0 == errorPath.stat() && 0 == errorPath.access(R_OK) &&
	    errorPath.isFile()) {
		this->_errorPage.open(errorPath.string().c_str(), std::ios::in | std::ios::binary);
		if (this->_errorPage.is_open()) {
			this->_context.response.setHeader(HEADER_CONTENT_LENGTH,
			                                  ft::numToStr(errorPath.size()));
			return REQ_DONE;
			UNSET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		}
	}
	return REQ_ERROR;
}

void Client::_loadErrorPage(void) {
	if (this->_openErrorPage() == REQ_ERROR) {
		std::string errorBody;
		errorBody = HTMLERROR(ft::numToStr(this->_context.response.statusCode()),
		                      statusCodeToMsg(this->_context.response.statusCode()));
		this->_context.response.setBody(errorBody);
		this->_context.response.setHeader(HEADER_CONTENT_LENGTH, ft::numToStr(errorBody.length()));
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
	}
}

void Client::_readErrorPage(void) {
	uint8_t buffer[REQ_BUFFER_SIZE];

	this->_errorPage.read((char *)buffer, REQ_BUFFER_SIZE);
	ssize_t bytes = this->_errorPage.gcount();
	if (bytes == 0) {
		SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		return;
	}

	this->_context.responseBuffer.append(buffer, bytes);
	return;
}
