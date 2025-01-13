#include <iostream>

#include "RequestPUT.hpp"

/* CONSTRUCTORS ************************************************************* */

// RequestPUT::RequestPUT(void)
// {
// 	// std::cerr << "RequestPUT created" << std::endl;
// }

RequestPUT::RequestPUT(RequestContext_t &context) : ARequest(context)
{
	std::cerr << "RequestPUT created" << std::endl;
}

RequestPUT::RequestPUT(const RequestPUT &other) : ARequest(other)
{
	// std::cerr << "RequestPUT copy" << std::endl;
	*this = other;
}

RequestPUT::~RequestPUT(void)
{
	// std::cerr << "RequestPUT destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestPUT	&RequestPUT::operator=(const RequestPUT &other)
{
	std::cerr << "RequestPUT assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

error_t	RequestPUT::parse(void)
{
	std::cerr << "RequestPUT parse" << std::endl;
	return (REQ_DONE);
}

error_t	RequestPUT::processIn(void)
{
	std::cerr << "RequestPUT processIn" << std::endl;
	// debug start
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setBody("Hello, World!");
	SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
	// debug end
	return (REQ_DONE);
}

error_t	RequestPUT::processOut(void)
{
	std::cerr << "RequestPUT processOut" << std::endl;
	return (REQ_DONE);
}

ARequest	*RequestPUT::clone(void) const
{
	std::cerr << "RequestPUT clone" << std::endl;
	return (new RequestPUT(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest	*createRequestPUT(RequestContext_t &context)
{
	std::cerr << "createRequestPUT" << std::endl;
	return (new RequestPUT(context));
}
