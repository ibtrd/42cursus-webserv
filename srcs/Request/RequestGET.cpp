#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>

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

void RequestGET::_openFile(void)
{
	std::cerr << "RequestGET _openFile" << std::endl;
	this->_fd = open(this->_path.string().c_str(), O_RDONLY);
	if (this->_fd == -1) {
		std::cerr << "open: " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(INTERNAL_SERVER_ERROR);
	}
}

/* ************************************************************************** */

error_t	RequestGET::parse(void)
{
	std::cerr << "RequestGET parse" << std::endl;
	SET_REQ_READ_BODY_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

error_t	RequestGET::process(void)
{
	std::cerr << "RequestGET process" << std::endl;

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
		this->_openFile();
		SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	if (!this->_path.isDir()) {
		this->_context.response.setStatusCode(CONFLICT);
		SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	if (!this->_path.isDirFormat()) {
		this->_context.response.setStatusCode(MOVED_PERMANENTLY);
		this->_context.response.setHeader(HEADER_LOCATION, this->_context.target + '/');
		SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}


	for (std::vector<std::string>::const_iterator it = this->_context.ruleBlock->indexes().begin(); it != this->_context.ruleBlock->indexes().end(); ++it) {
		std::string test = this->_path.concat(*it);
		if (0 == access(test.c_str(), F_OK)) {
			this->_path = test;
			break ;
		} 
	}

	/* to implement: index */
	// iterate over index files (exists)
	// if found:
	//  if not readable -> 401
	//  if not regular file -> 409
	//  open file

	this->_context.response.setStatusCode(OK);
	this->_context.response.setBody("Directory listing not implemented");
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
