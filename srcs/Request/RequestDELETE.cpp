#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

#include "RequestDELETE.hpp"
#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

// RequestDELETE::RequestDELETE(void)
// {
// 	// std::cerr << "RequestDELETE created" << std::endl;
// }

RequestDELETE::RequestDELETE(RequestContext_t &context) : ARequest(context)
{
	std::cerr << "RequestDELETE created" << std::endl;
}

RequestDELETE::RequestDELETE(const RequestDELETE &other) : ARequest(other)
{
	// std::cerr << "RequestDELETE copy" << std::endl;
	*this = other;
}

RequestDELETE::~RequestDELETE(void)
{
	// std::cerr << "RequestDELETE destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestDELETE	&RequestDELETE::operator=(const RequestDELETE &other)
{
	std::cerr << "RequestDELETE assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

error_t	RequestDELETE::parse(void)
{
	std::cerr << "RequestDELETE parse" << std::endl;
	SET_REQ_READ_BODY_COMPLETE(this->_context.requestState);
	std::cout << ((LocationBlock *)this->_context.ruleBlock)->getRoot().concat(this->_context.target) << std::endl;
	return (REQ_DONE);
}

error_t	RequestDELETE::processIn(void)
{
	std::cerr << "RequestDELETE processIn" << std::endl;
	if (0 != this->_path.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (0 != this->_path.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	// if (!this->_path.hasPermission(R_OK)) {
	// 	this->_context.response.setStatusCode(STATUS_FORBIDDEN);
	// 	SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
	// 	return (REQ_DONE);
	// }
	if (!this->_path.isDir()) {
		if (0 != std::remove(this->_path.c_str())) {
			this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		} else {
			this->_context.response.setStatusCode(STATUS_NO_CONTENT);
		}
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (this->_path.isDirFormat()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (0 != this->_path.access(W_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (0 != std::remove(this->_path.c_str())) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
	} else {
		this->_context.response.setStatusCode(STATUS_NO_CONTENT);
	}
	SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

error_t	RequestDELETE::processOut(void)
{
	std::cerr << "RequestDELETE processOut" << std::endl;
	return (REQ_DONE);
}

ARequest	*RequestDELETE::clone(void) const
{
	std::cerr << "RequestDELETE clone" << std::endl;
	return (new RequestDELETE(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest	*createRequestDELETE(RequestContext_t &context)
{
	std::cerr << "createRequestDELETE" << std::endl;
	return (new RequestDELETE(context));
}
