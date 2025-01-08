#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

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
	return (REQ_DONE);
}

error_t	RequestDELETE::process(void)
{
	std::cerr << "RequestDELETE process" << std::endl;
	// debug start
	this->_context.response.setStatusCode(OK);
	this->_context.response.setBody("Hello, World!");
	this->_context.responseBuffer = this->_context.response.response();
	SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
	// debug end
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
