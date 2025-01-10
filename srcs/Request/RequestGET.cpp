#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
# include <sys/stat.h>

#include "RequestGET.hpp"
#include "ft.hpp"
#include "Client.hpp"

/* CONSTRUCTORS ************************************************************* */

// RequestGET::RequestGET(void)
// {
// 	// std::cerr << "RequestGET created" << std::endl;
// }

RequestGET::RequestGET(RequestContext_t &context) : ARequest(context)
{
	std::cerr << "RequestGET created" << std::endl;
}

RequestGET::RequestGET(const RequestGET &other) : ARequest(other)
{
	// std::cerr << "RequestGET copy" << std::endl;
	*this = other;
}

RequestGET::~RequestGET(void)
{
	// std::cerr << "RequestGET destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestGET	&RequestGET::operator=(const RequestGET &other)
{
	std::cerr << "RequestGET assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

error_t	RequestGET::parse(void)
{
	std::cerr << "RequestGET parse" << std::endl;
	SET_REQ_READ_BODY_COMPLETE(this->_context.requestState);	// GET requests have no body
	// temporary code
	LocationBlock *block = (LocationBlock *)this->_context.ruleBlock;
	this->_path = block->getRoot().string() + this->_context.target;
	// 
	return (REQ_DONE);
}

error_t	RequestGET::process(void)
{
	std::cerr << "RequestGET process" << std::endl;

	// LocationBlock	*block = (LocationBlock *)this->_context.ruleBlock;

	// std::string		path(block->getRoot().string());	// debug

	// path += this->_context.target;

	// struct stat		buf;
	// if (stat(path.c_str(), &buf))	// Error handling
	// {
	// 	this->_context.response.setStatusCode(I_AM_A_TEAPOT);
	// 	this->_context.responseBuffer = this->_context.response.response();
	// 	SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
	// 	return (REQ_DONE);
	// }

	// if (S_ISDIR(buf.st_mode))
	// {
	// 	this->_context.response.setStatusCode(OK);
	// 	this->_context.response.setBody("Content of directory");
	// 	this->_context.responseBuffer = this->_context.response.response();
	// 	SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
	// 	return (REQ_DONE);
	// }

	// if (S_ISREG(buf.st_mode))
	// {
	// 	this->_context.response.setStatusCode(OK);
	// 	this->_context.response.setBody("Content of file");
	// 	this->_context.responseBuffer = this->_context.response.response();
	// 	SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
	// 	return (REQ_DONE);
	// }

	// // debug 418 start
	// this->_context.response.setStatusCode(I_AM_A_TEAPOT);
	// // this->_context.response.setBody("Hello, World!");
	// this->_context.responseBuffer = this->_context.response.response();
	// SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
	// // debug end

	// // debug 200 start
	// this->_context.response.setStatusCode(OK);
	// this->_context.response.setBody("Hello, World!");
	// this->_context.responseBuffer = this->_context.response.response();
	// SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
	// // debug end

	return (REQ_DONE);
}

ARequest	*RequestGET::clone(void) const
{
	std::cerr << "RequestGET clone" << std::endl;
	return (new RequestGET(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest	*createRequestGET(RequestContext_t &context)
{
	std::cerr << "createRequestGET" << std::endl;
	return (new RequestGET(context));
}
