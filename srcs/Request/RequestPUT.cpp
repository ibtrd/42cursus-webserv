#include "RequestPUT.hpp"
#include "ft.hpp"

#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

/* CONSTRUCTORS ************************************************************* */

// RequestPUT::RequestPUT(void)
// {
// 	// std::cerr << "RequestPUT created" << std::endl;
// }

RequestPUT::RequestPUT(RequestContext_t &context) : ARequest(context), _chunked(false), _contentLength(0) {
	// std::cerr << "RequestPUT created" << std::endl;
	SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);	// No workOut needed
}

RequestPUT::RequestPUT(const RequestPUT &other) : ARequest(other), _chunked(false), _contentLength(0) {
	// std::cerr << "RequestPUT copy" << std::endl;
	*this = other;
}

RequestPUT::~RequestPUT(void) {
	// std::cerr << "RequestPUT destroyed" << std::endl;
	if (this->_file.is_open()) this->_file.close();
	if (0 == this->_tmpFilename.access(F_OK)) std::remove(this->_tmpFilename.c_str());
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestPUT &RequestPUT::operator=(const RequestPUT &other) {
	// std::cerr << "RequestPUT assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

error_t RequestPUT::_generateFilename(void) {
	std::string tmp;
	int32_t i = 0;
	do {
		tmp = this->_path.string();
		tmp += ".";
		tmp += ft::generateRandomString(8);
		tmp += ".tmp";
		++i;
	} while (0 == access(tmp.c_str(), F_OK) && i < 100);
	if (i == 100) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return (REQ_DONE);
	}
	this->_tmpFilename = tmp;
	return (REQ_CONTINUE);
}

void RequestPUT::_openFile(void) {
	if (REQ_CONTINUE != this->_generateFilename()) return;
	this->_file.open(this->_tmpFilename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	if (!this->_file.is_open()) {
		std::cerr << "open(): " << std::strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	}
}

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
		this->_contentLength = sToContentLength(it->second);
		if (this->_contentLength == CONTENT_LENGTH_INVALID) {
			this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
			return (REQ_DONE);
		}
		if (this->_contentLength == CONTENT_LENGTH_TOO_LARGE || this->_contentLength > this->_context.ruleBlock->getMaxBodySize()) {
			this->_context.response.setStatusCode(STATUS_PAYLOAD_TOO_LARGE);
			return (REQ_DONE);
		}
		if (this->_contentLength == 0) {
			this->_context.response.setStatusCode(STATUS_NO_CONTENT);
			return (REQ_DONE);
		}
	}
	return (REQ_CONTINUE);
}

error_t	RequestPUT::_readContent(void) {
	if (this->_contentLength - static_cast<int32_t>(this->_context.buffer.size()) >= 0) {
		this->_file.write(this->_context.buffer.c_str(), this->_context.buffer.size());
		this->_contentLength -= this->_context.buffer.size();
		this->_context.buffer.clear();
		// std::cerr << ".";	// DEBUG
	} else {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (this->_contentLength == 0) {
		this->_file.close();
		if (0 == this->_path.access(F_OK)) {
			if (0 != std::remove(this->_path.c_str())) {
				this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
			}
		}
		if (0 != std::rename(this->_tmpFilename.c_str(), this->_path.c_str())) {
			this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		}
		std::remove(this->_tmpFilename.c_str());
		this->_context.response.setStatusCode(STATUS_CREATED);
		SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	return (REQ_CONTINUE);
}
/*
AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEFFFFFFFFFFGGGGGGGGGGHHHHHHHHHHIIIIIIIIIIJJJJJJJJJJKKKKKKKKKKLLLLLLLLLMMMMMMMMMNNNNNNNNNOOOOOOOOOPPPPPPPPP
*/
error_t RequestPUT::_readChunked(void) {
	std::cerr << "RequestPUT _readChunked" << std::endl;
	this->_context.response.setStatusCode(STATUS_NOT_IMPLEMENTED);
	SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

/* ************************************************************************** */

void RequestPUT::processing(void) {
	// std::cerr << "RequestPUT parse" << std::endl;

	// Check headers
	if (REQ_CONTINUE != this->_checkHeaders()) return;

	// Check path
	if (this->_path.isDirFormat()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		return;
	}
	else if (0 == this->_path.access(F_OK)) {
		if (0 == this->_path.access(W_OK)) {
			this->_openFile();
		} else {
			this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		}
		return;
	}
	Path parent = this->_path.dir();
	if (0 != parent.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
		return;
	}
	if (0 != parent.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	if (!parent.isDir()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		return;
	}
	if (0 != parent.access(W_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		return;
	}

	this->_openFile();
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

error_t RequestPUT::workOut(void) {
	throw std::logic_error("RequestPUT::workOut should not be called");
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
