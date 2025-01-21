#include "RequestHEAD.hpp"

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "Client.hpp"
#include "Server.hpp"
#include "ft.hpp"
#include "webservHTML.hpp"

/* CONSTRUCTORS ************************************************************* */

RequestHEAD::RequestHEAD(RequestContext_t &context) : ARequest(context), _dir(NULL) {
	std::cerr << "RequestHEAD created" << std::endl;
	SET_REQ_WORK_COMPLETE(this->_context.requestState);  // No work needed
}

RequestHEAD::RequestHEAD(const RequestHEAD &other) : ARequest(other), _dir(NULL) {
	std::cerr << "RequestHEAD copy" << std::endl;
	*this = other;
}

RequestHEAD::~RequestHEAD(void) {
	std::cerr << "RequestHEAD destroyed" << std::endl;
	if (this->_file.is_open()) {
		this->_file.close();
	}
	if (this->_dir != NULL) {
		closedir(this->_dir);
	}
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestHEAD &RequestHEAD::operator=(const RequestHEAD &other) {
	std::cerr << "RequestHEAD assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

void RequestHEAD::_openFile(void) {
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setHeader(HEADER_CONTENT_TYPE,
	                                  this->_context.server.getMimeType(this->_path.extension()));
	this->_context.response.setHeader(HEADER_CONTENT_LENGTH, ft::numToStr(this->_path.size()));
}

void RequestHEAD::_openDir(void) {
	// std::cerr << "RequestHEAD _openDir" << std::endl;
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setHeader(HEADER_CONTENT_TYPE, "text/html");
}

error_t RequestHEAD::_validateLocalFile(void) {
	if (0 != this->_path.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (0 != this->_path.access(R_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (this->_path.isFile()) {
		this->_openFile();
		SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	return (REQ_CONTINUE);
}

error_t RequestHEAD::_fetchIndexes(void) {
	for (std::vector<std::string>::const_iterator it = this->_context.ruleBlock->indexes().begin();
	     it != this->_context.ruleBlock->indexes().end(); ++it) {
		std::string test = this->_path.concat(*it);
		// std::cerr << "testing indexfile: " << test << std::endl;
		if (0 == access(test.c_str(), F_OK)) {
			this->_path = test;
			return 0;
		}
	}
	return -1;
}

/* ************************************************************************** */

void RequestHEAD::processing(void) {
	// std::cerr << "RequestHEAD parse" << std::endl;
	if (0 != this->_path.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
		return;
	}
	if (REQ_CONTINUE != this->_validateLocalFile()) {
		return;
	}
	if (!this->_path.isDir()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		return;
	}
	if (!this->_path.isDirFormat()) {
		this->_context.response.setStatusCode(STATUS_MOVED_PERMANENTLY);
		this->_context.response.setHeader(HEADER_LOCATION, this->_context.target + '/');
		return;
	}
	if (0 == this->_fetchIndexes()) {
		if (REQ_CONTINUE != this->_validateLocalFile()) {
			return;
		}
	} else {
		if (this->_context.ruleBlock->isDirListing()) {
			this->_openDir();
			return;
		}
	}
	this->_context.response.setStatusCode(STATUS_FORBIDDEN);
}

error_t RequestHEAD::workIn(void) {
	throw std::logic_error("RequestHEAD::workIn should not be called");
	return (REQ_DONE);
}

error_t RequestHEAD::workOut(void) {
	throw std::logic_error("RequestHEAD::workOut should not be called");
	return (REQ_ERROR);
}

ARequest *RequestHEAD::clone(void) const {
	std::cerr << "RequestHEAD clone" << std::endl;
	return (new RequestHEAD(*this));
}

/* HEADTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest *createRequestHEAD(RequestContext_t &context) {
	std::cerr << "createRequestHEAD" << std::endl;
	return (new RequestHEAD(context));
}
