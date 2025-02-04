#include "RequestDELETE.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

// RequestDELETE::RequestDELETE(void)
// {
// 	// std::cerr << "RequestDELETE created" << std::endl;
// }

RequestDELETE::RequestDELETE(RequestContext_t &context) : ARequest(context) {
	std::cerr << "RequestDELETE created" << std::endl;
	SET_REQ_WORK_COMPLETE(this->_context.requestState);  // No work needed
	// SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
}

RequestDELETE::RequestDELETE(const RequestDELETE &other) : ARequest(other) {
	// std::cerr << "RequestDELETE copy" << std::endl;
	*this = other;
}

RequestDELETE::~RequestDELETE(void) {
	// std::cerr << "RequestDELETE destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestDELETE &RequestDELETE::operator=(const RequestDELETE &other) {
	std::cerr << "RequestDELETE assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

void RequestDELETE::processing(void) {
	std::cerr << "RequestDELETE processing" << std::endl;

	std::cout
	    << ((LocationBlock *)this->_context.ruleBlock)->getRoot().concat(this->_context.target)
	    << std::endl;

	std::cerr << this->_path << std::endl;

	if (0 != this->_path.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
		return;
	}
	if (0 != this->_path.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	if (this->_path.isDir()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		return;
	}
	if (0 != this->_path.access(W_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		return;
	}
	if (0 != std::remove(this->_path.c_str())) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	} else {
		this->_context.response.setStatusCode(STATUS_NO_CONTENT);
	}
	return;
}

ARequest *RequestDELETE::clone(void) const { return (new RequestDELETE(*this)); }

/* OTHERS *********************************************************************/

ARequest *createRequestDELETE(RequestContext_t &context) {
	std::cerr << "createRequestDELETE" << std::endl;
	return (new RequestDELETE(context));
}
