#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <iostream>

#include "Client.hpp"

/* STATIC VARIABLES ********************************************************* */

const char *ARequest::_chunkTerminator[CHUNK_TERMINATOR_SIZE] = {"0\r\n\r\n", "0\r\n\r", "0\r\n",
                                                                   "0\r", "0"};

/* CONSTRUCTORS ************************************************************* */

// ARequest::ARequest(void)
// {
// 	// std::cerr << "ARequest created" << std::endl;
// }

ARequest::ARequest(RequestContext_t &context) : _context(context) {
	// std::cerr << "ARequest created" << std::endl;
	uint32_t matchLength = this->_context.ruleBlock->path().string().size() - 1;
	std::string chopedTarget = this->_context.target.substr(matchLength, std::string::npos);
	const Path &root = this->_context.ruleBlock->getRoot().string();
	
	if (!chopedTarget.empty()) {
		this->_path = root.concat(chopedTarget);
	} else {
		this->_path = root.string().substr(0, root.string().size() - 1);
	}

	this->_cgiPath = this->_context.ruleBlock->findCGI(this->_path.extension());
}

ARequest::ARequest(const ARequest &other) : _context(other._context) {
	// std::cerr << "ARequest copy" << std::endl;
	uint32_t matchLength = this->_context.ruleBlock->path().string().size() - 1;
	std::string chopedTarget = this->_context.target.substr(matchLength, std::string::npos);
	const Path &root = this->_context.ruleBlock->getRoot().string();
	
	if (!chopedTarget.empty()) {
		this->_path = root.concat(chopedTarget);
	} else {
		this->_path = root.string().substr(0, root.string().size() - 1);
	}

	this->_cgiPath = this->_context.ruleBlock->findCGI(this->_path.extension());
}

ARequest::~ARequest(void) {
	// std::cerr << "ARequest destroyed" << std::endl;
}

/* OPERATOR OVERLOADS *****************status************************************** */

ARequest &ARequest::operator=(const ARequest &other) {
	std::cerr << "ARequest assign" << std::endl;
	if (this == &other) {
		return (*this);
	}
	// this->_context = other._context;
	return (*this);
}

/* ************************************************************************** */

error_t ARequest::_readCGI(void) {
	uint8_t buffer[REQ_BUFFER_SIZE];

	static int compteur = 0;

	std::cerr << "Request _readCGI: " << compteur++ << std::endl;

	ssize_t bytes = read(this->_context.cgiSockets[PARENT_SOCKET], buffer, REQ_BUFFER_SIZE);
	if (bytes == 0) {
		std::cerr << "read: EOF" << std::endl;
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
		return (REQ_CONTINUE);
	}
	if (bytes == -1) {
		std::cerr << "read: " << strerror(errno) << std::endl;
		// this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
		return (REQ_ERROR);
	}

	std::cerr << "Request _readCGI: " << bytes << std::endl;
	std::cerr << "Request _readCGI: |";
	for (ssize_t i = 0; i < bytes; ++i) {
		std::cerr << buffer[i];
	}
	std::cerr << "|" << std::endl;
	BinaryBuffer bBuffer(buffer, bytes);
	if (!IS_REQ_CGI_HEADERS_COMPLETE(this->_context.requestState)) {
		this->_parseCGIHeaders(bBuffer);
	}
	this->_context.responseBuffer.append(bBuffer);
	return (REQ_CONTINUE);
}

static status_code_t cgiStatusToStatus(std::string &statusHeader) {
	if (statusHeader.size() < 5 && statusHeader[3] == ' ' && std::isalpha(statusHeader[4])) {
		return (STATUS_INTERNAL_SERVER_ERROR);
	}
	if (!std::isdigit(statusHeader[0]) || !std::isdigit(statusHeader[1]) || !std::isdigit(statusHeader[2])) {
		return (STATUS_INTERNAL_SERVER_ERROR);
	}

	status_code_t code = (statusHeader[0] - '0') * 100 + (statusHeader[1] - '0') * 10 + (statusHeader[2] - '0');
	statusHeader.erase(0, 4);
	return (code);
}

void ARequest::_parseCGIHeaders(BinaryBuffer &buffer) {
	size_t      pos;
	std::string line;
	std::string key;
	std::string value;

	// std::cerr << "Parsing headers..." << std::endl;
	while ((pos = buffer.find("\r\n")) != std::string::npos) {
		line = buffer.substr(0, pos);
		buffer.erase(0, pos + 2);
		if (line.empty()) {
			headers_t::iterator statusHeader = this->_context.response.header(HEADER_STATUS);
			if (statusHeader == this->_context.response.headersEnd()) {
				this->_context.response.setStatusCode(STATUS_OK);
				// SET_REQ_READ_COMPLETE(this->_context.requestState);
			} else {
				this->_context.response.setStatusCode(cgiStatusToStatus(statusHeader->second));
				this->_context.response.setReasonPhrase(statusHeader->second);
				this->_context.response.deleteHeader(HEADER_STATUS);
			}
			SET_REQ_CGI_HEADERS_COMPLETE(this->_context.requestState);
			this->_context.response.clearBody();
			this->_context.responseBuffer = this->_context.response.response();
			// return (REQ_DONE);
			return;
		}
		pos = line.find(": ");
		if (pos == std::string::npos) {
			SET_REQ_CGI_HEADERS_COMPLETE(this->_context.requestState);
			SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
			this->_context.response.clear();
			this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
			this->_context.responseBuffer = this->_context.response.response();
			// return (REQ_DONE);
			return;
		}
		key                         = line.substr(0, pos);
		value                       = line.substr(pos + 2);
		this->_context.response.setHeader(key, value);
		std::cerr << "cgiHeader: |" << key << "| |" << value << "|" << std::endl;
	}
	// return (REQ_CONTINUE);
	return;
}

/* ************************************************************************** */

error_t ARequest::workIn(void) {
	throw std::logic_error("ARequest::workIn should not be called");
	return (REQ_DONE);
}

error_t ARequest::workOut(void) {
	throw std::logic_error("ARequest::workOut should not be called");
	return (REQ_ERROR);
}

error_t ARequest::CGIIn(void) {
	throw std::logic_error("ARequest::CGIIn should not be called");
	return (REQ_ERROR);
}

error_t ARequest::CGIOut(void) {
	throw std::logic_error("ARequest::CGIOut should not be called");
	return (REQ_ERROR);
}

/* GETTERS ****************************************************************** */

const RequestContext_t &ARequest::context(void) const {
	return this->_context;
}

const Path &ARequest::path(void) const {
	return this->_path;
}
const Path &ARequest::cgiPath(void) const {
	return *this->_cgiPath;
}

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */
