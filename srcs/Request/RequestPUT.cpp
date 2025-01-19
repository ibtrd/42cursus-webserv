#include "RequestPUT.hpp"

#include <cstring>
#include <iostream>
#include <fcntl.h>

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
	if (this->_path.isDirFormat()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (0 == this->_path.access(F_OK)) {
		if (0 == this->_path.access(W_OK)) {
			this->_openFile(this->_path.c_str());
		} else {
			this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		}
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	Path parent = this->_path.dir();
	if (0 != parent.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (0 != parent.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (!parent.isDir()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (0 != parent.access(W_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	this->_openFile(this->_path.c_str());
	// SET_REQ_READ_BODY_COMPLETE(this->_context.requestState);
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

/* ************************************************************************** */

void RequestPUT::_openFile(const char *filepath) {
	this->_file.open(filepath, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!this->_file.is_open()) {
		std::cerr << "open(): " << std::strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	}
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest *createRequestPUT(RequestContext_t &context) {
	std::cerr << "createRequestPUT" << std::endl;
	return (new RequestPUT(context));
}
