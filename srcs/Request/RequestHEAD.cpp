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
	SET_REQ_WORK_COMPLETE(this->_context.requestState);
}

RequestHEAD::RequestHEAD(const RequestHEAD &other) : ARequest(other), _dir(NULL) { *this = other; }

RequestHEAD::~RequestHEAD(void) {
	if (this->_file.is_open()) {
		this->_file.close();
	}
	if (this->_dir != NULL) {
		closedir(this->_dir);
	}
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestHEAD &RequestHEAD::operator=(const RequestHEAD &other) {
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
		if (0 == access(test.c_str(), F_OK)) {
			this->_path = test;
			return 0;
		}
	}
	return -1;
}

/* ************************************************************************** */

void RequestHEAD::processing(void) {
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
		this->_context.response.setHeader(
		    HEADER_LOCATION,
		    this->_context.target + '/' + this->_context.queries.originalQueryLine());
		return;
	}

	if (0 == this->_fetchIndexes() && REQ_CONTINUE != this->_validateLocalFile()) {
		return;
	} else if (this->_context.ruleBlock->isDirListing()) {
		this->_openDir();
		return;
	}
	this->_context.response.setStatusCode(STATUS_FORBIDDEN);
}

ARequest *RequestHEAD::clone(void) const { return (new RequestHEAD(*this)); }

/* OTHERS *********************************************************************/

ARequest *createRequestHEAD(RequestContext_t &context) { return (new RequestHEAD(context)); }
