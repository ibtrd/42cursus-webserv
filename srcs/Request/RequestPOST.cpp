#include "RequestPOST.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

// RequestPOST::RequestPOST(void)
// {
// 	// std::cerr << "RequestPOST created" << std::endl;
// }

RequestPOST::RequestPOST(RequestContext_t &context) : ARequest(context) {
	std::cerr << "RequestPOST created" << std::endl;
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
	std::cerr << "RequestPOST assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

void RequestPOST::processing(void) {
	std::cerr << "RequestPOST parse" << std::endl;
}

error_t RequestPOST::workIn(void) {
	std::cerr << "RequestPOST workIn" << std::endl;
	// debug start
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setBody("Hello, World!");
	SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
	// debug end
	return (REQ_DONE);
}

error_t RequestPOST::workOut(void) {
	std::cerr << "RequestPOST workOut" << std::endl;
	SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

ARequest *RequestPOST::clone(void) const {
	std::cerr << "RequestPOST clone" << std::endl;
	return (new RequestPOST(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest *createRequestPOST(RequestContext_t &context) {
	std::cerr << "createRequestPOST" << std::endl;
	return (new RequestPOST(context));
}
