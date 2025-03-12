
#include "RequestContext.hpp"
#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

Response::Response(void) {
	this->_statusCode             = STATUS_NONE;
	this->_reasonPhrase           = "";
	this->_headers[HEADER_SERVER] = WEBSERV_VERSION;
};

Response::Response(const Response &other) { *this = other; }

Response::~Response(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Response &Response::operator=(const Response &other) {
	if (this == &other) {
		return (*this);
	}
	this->_statusCode   = other._statusCode;
	this->_reasonPhrase = other._reasonPhrase;
	this->_headers      = other._headers;
	return (*this);
}

/* ************************************************************************** */

/* GETTERS ****************************************************************** */

status_code_t Response::statusCode(void) const { return (this->_statusCode); }

std::string Response::reasonPhrase(void) const { return (this->_reasonPhrase); }

std::string Response::statusLine(void) const {
	return ("HTTP/1.1 " + ft::numToStr(this->_statusCode) + " " + this->_reasonPhrase + "\r\n");
}

headers_t::iterator Response::header(const std::string &key) { return (this->_headers.find(key)); }

std::string Response::body(void) const { return (this->_body); }

std::string Response::response(void) const {
	std::string response = this->statusLine();
	for (headers_t::const_iterator it = this->_headers.begin(); it != this->_headers.end(); it++) {
		response += it->first + ": " + it->second + "\r\n";
	}
	response += "\r\n";
	response += this->_body;
	return (response);
}

size_t Response::bodySize(void) const { return (this->_body.size()); }

headers_t::const_iterator Response::headersBegin(void) const { return (this->_headers.begin()); }

headers_t::const_iterator Response::headersEnd(void) const { return (this->_headers.end()); }

/* SETTERS ****************************************************************** */

void Response::setStatusCode(const status_code_t &code) {
	this->_statusCode   = code;
	this->_reasonPhrase = statusCodeToReason(code);
}

void Response::setHeader(const std::string &key, const std::string &value) {
	this->_headers[key] = value;
}

void Response::setReasonPhrase(const std::string &reasonPhrase) {
	this->_reasonPhrase = reasonPhrase;
}

void Response::setBody(const std::string &body) { this->_body = body; }

void Response::addBody(const std::string &body) { this->_body += body; }

void Response::deleteHeader(const std::string &key) { this->_headers.erase(key); }

void Response::clearBody(void) { this->_body.clear(); }

void Response::clearHeaders(void) { this->_headers.clear(); }

void Response::clear(void) {
	this->_statusCode   = STATUS_NONE;
	this->_reasonPhrase = "";
	this->_headers.clear();
	this->_body.clear();
}

/* EXCEPTIONS *************************************************************** */
