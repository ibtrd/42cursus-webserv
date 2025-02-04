#include "Server.hpp"

error_t Client::_parseRequest(void) {
	error_t ret;

	// Parse request line
	if (!IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_context.requestState)) {
		ret = this->_parseRequestLine();
		if (ret != REQ_DONE) {
			return (ret);
		}
	}

	// Parse headers
	if (!IS_REQ_READ_HEADERS_COMPLETE(this->_context.requestState)) {
		ret = this->_parseHeaders();
		if (ret != REQ_DONE) {
			return (ret);
		}
	}

	// DEBUG (buffer)

	if (this->_context.response.statusCode() == STATUS_NONE) {
		ret = this->_resolveARequest();
		if (ret != REQ_CONTINUE) {
			return (ret);
		}
	}

	SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

error_t Client::_parseRequestLine(void) {
	std::string requestLine;

	// Check at least one line is present
	size_t pos = this->_context.buffer.find("\r\n");
	if ((pos == std::string::npos && this->_context.buffer.size() > REQUEST_LINE_LIMIT) ||
	    (pos != std::string::npos && pos > REQUEST_LINE_LIMIT)) {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (pos == std::string::npos) {
		return (REQ_CONTINUE);
	}
	requestLine = this->_context.buffer.substr(0, pos);
	this->_context.buffer.erase(0, pos + 2);

	// Parse request line

	// Method
	pos = requestLine.find(' ');
	if (pos == std::string::npos) {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	std::string method = requestLine.substr(0, pos);
	if (method.empty()) {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	this->_context.method = Method(method);
	requestLine.erase(0, pos + 1);
	if (!this->_context.method.isValid()) {
		this->_context.response.setStatusCode(STATUS_METHOD_NOT_ALLOWED);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	// Target
	pos = requestLine.find(' ');
	if (pos == std::string::npos) {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	this->_context.target = requestLine.substr(0, pos);
	if (this->_context.target.empty() || this->_context.target[0] != '/') {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	requestLine.erase(0, pos + 1);

	//  Query string
	pos = this->_context.target.find('?');
	if (pos != std::string::npos) {
		this->_context.queries = Queries(this->_context.target.substr(pos + 1));
		this->_context.target.erase(pos);
	}

	// Protocol version
	this->_context.protocolVersion = requestLine;
	if (this->_context.protocolVersion.empty()) {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (this->_context.protocolVersion != HTTP_PROTOCOL_VERSION) {
		this->_context.response.setStatusCode(STATUS_HTTP_VERSION_NOT_SUPPORTED);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	SET_REQ_READ_REQUEST_LINE_COMPLETE(this->_context.requestState);

	return (REQ_DONE);
}

error_t Client::_parseHeaders(void) {
	size_t      pos;
	std::string line;
	std::string key;
	std::string value;

	// std::cerr << "Parsing headers..." << std::endl;
	while ((pos = this->_context.buffer.find("\r\n")) != std::string::npos) {
		line = this->_context.buffer.substr(0, pos);
		this->_context.buffer.erase(0, pos + 2);
		if (line.empty()) {
			if (this->_context.headers.find(HEADER_HOST) == this->_context.headers.end()) {
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_READ_COMPLETE(this->_context.requestState);
			} else {
				SET_REQ_READ_HEADERS_COMPLETE(this->_context.requestState);
			}
			return (REQ_DONE);
		}
		pos = line.find(": ");
		if (pos == std::string::npos) {
			this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
			SET_REQ_READ_COMPLETE(this->_context.requestState);
			return (REQ_DONE);
		}
		key                         = line.substr(0, pos);
		value                       = line.substr(pos + 2);
		this->_context.headers[key] = value;
	}
	return (REQ_CONTINUE);
}
