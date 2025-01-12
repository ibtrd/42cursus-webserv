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
	// LocationBlock *block = (LocationBlock *)this->_context.ruleBlock;
	// this->_path = block->getRoot().concat(this->_context.target);
	// std::cerr << "Path: " << this->_path << std::endl;
	//
	// if (access(this->_path.c_str(), F_OK) == -1) {
	// 	this->_context.response.setStatusCode(NOT_FOUND);
	// 	SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
	// 	return (REQ_DONE);
	// }
	return (REQ_DONE);
}

# include <fcntl.h>
# include <dirent.h>
# include "Server.hpp"

error_t	RequestGET::process(void)
{
	std::cerr << "RequestGET process" << std::endl;

	std::cerr << "EXIST: " << this->_path.string() << std::endl;
	if (!this->_path.exists()) {
		this->_context.response.setStatusCode(NOT_FOUND);
		SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	if (this->_path.hasPermission(R_OK) == false) {
		this->_context.response.setStatusCode(UNAUTHORIZED);
		SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	if (this->_path.isFile()) {
		this->_context.response.setStatusCode(OK);
		this->_context.response.setBody(this->_path.string());
		SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	if (this->_path.isDir()) {
		this->_context.response.setStatusCode(OK);
		this->_context.response.setBody("Directory listing not implemented");
		SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	this->_context.response.setStatusCode(CONFLICT);
	SET_REQ_PROCESS_COMPLETE(this->_context.requestState);

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
