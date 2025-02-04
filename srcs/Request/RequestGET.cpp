#include "RequestGET.hpp"

#include <unistd.h>

#include "CgiBuilder.hpp"
#include "Server.hpp"
#include "ft.hpp"
#include "webservHTML.hpp"

/* CONSTRUCTORS ************************************************************* */

RequestGET::RequestGET(RequestContext_t &context) : ARequest(context), _dir(NULL) {
	SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
	SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
	SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
}

RequestGET::RequestGET(const RequestGET &other) : ARequest(other), _dir(NULL) {
	*this = other;
}

RequestGET::~RequestGET(void) {
	if (this->_file.is_open()) {
		this->_file.close();
	}
	if (this->_dir != NULL) {
		closedir(this->_dir);
	}
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestGET &RequestGET::operator=(const RequestGET &other) {
	(void)other;
	return (*this);
}

/* PRIVATE METHODS ********************************************************** */

void RequestGET::_openFile(void) {
	this->_file.open(this->_path.c_str(), std::ios::in | std::ios::binary);
	if (!this->_file.is_open()) {
		std::cerr << "Error: open(): " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setHeader(HEADER_CONTENT_TYPE,
	                                  this->_context.server.getMimeType(this->_path.extension()));
	this->_context.response.setHeader(HEADER_CONTENT_LENGTH, ft::numToStr(this->_path.size()));
}

void RequestGET::_openDir(void) {
	this->_dir = opendir(this->_path.string().c_str());
	if (this->_dir == NULL) {
		std::cerr << "Error: opendir(): " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setHeader(HEADER_CONTENT_TYPE, "text/html");
	this->_context.response.setBody(INDEXOF(this->_context.target));
}

error_t RequestGET::_readFile(void) {
	uint8_t buffer[REQ_BUFFER_SIZE];

	this->_file.read((char *)buffer, REQ_BUFFER_SIZE);
	ssize_t bytes = this->_file.gcount();
	if (bytes == 0) {
		SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		return (REQ_CONTINUE);
	}

	this->_context.responseBuffer.append(buffer, bytes);
	return (REQ_CONTINUE);
}

error_t RequestGET::_fetchIndexes(void) {
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

error_t RequestGET::_validateLocalFile(void) {
	if (0 != this->_path.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return (REQ_DONE);
	}
	if (0 != this->_path.access(R_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		return (REQ_DONE);
	}
	if (this->_path.isFile()) {
		if (this->_cgiPath) {
			this->_openCGI();
		} else {
			this->_openFile();
		}
		return (REQ_DONE);
	}
	return (REQ_CONTINUE);
}

/* PUBLIC METHODS *********************************************************** */

void RequestGET::processing(void) {
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

error_t RequestGET::workOut(void) {
	if (this->_path.isFileFormat()) {
		return (this->_readFile());
	}
	if (this->_path.isDirFormat()) {
		return (this->_readDir());
	}

	return (REQ_ERROR);
}

error_t RequestGET::CGIIn(void) {
	return (this->_readCGI());
}

ARequest *RequestGET::clone(void) const {
	return (new RequestGET(*this));
}

/* OTHERS *********************************************************************/

ARequest *createRequestGET(RequestContext_t &context) {
	return (new RequestGET(context));
}
