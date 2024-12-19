#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

#include "Response.hpp"

/* CONSTRUCTORS ************************************************************* */

Response::Response(void)
{
	// std::cerr << "Response created" << std::endl;
};

Response::Response(const Response &other)
{
	// std::cerr << "Response copy" << std::endl;
	*this = other;
}

Response::~Response(void)
{
	// std::cerr << "Response destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

Response	&Response::operator=(const Response &other)
{
	// std::cerr << "Response assign" << std::endl;
	if (this == &other)
		return (*this);
	this->_statusCode = other._statusCode;
	this->_reasonPhrase = other._reasonPhrase;
	this->_headers = other._headers;
	return (*this);
}

/* ************************************************************************** */



/* GETTERS ****************************************************************** */

StatusCode	Response::statusCode(void) const
{
	return (this->_statusCode);
}

std::string	Response::reasonPhrase(void) const
{
	return (this->_reasonPhrase);
}

std::string	Response::statusLine(void) const
{
	return ("HTTP/1.1 " + numToStr(this->_statusCode) + " " + this->_reasonPhrase + "\r\n");
}

std::string	Response::header(const std::string &key) const
{
	return (this->_headers.at(key));
}

std::string	Response::body(void) const
{
	return (this->_body);
}

std::string	Response::response(void) const
{
	std::string	response = this->statusLine();
	for (std::map<std::string, std::string>::const_iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
	{
		response += it->first + ": " + it->second + "\r\n";
	}
	response += "\r\n";
	response += this->_body;
	return (response);
}

/* SETTERS ****************************************************************** */

void	Response::setStatusCode(const StatusCode &code)
{
	this->_statusCode = code;
	this->_reasonPhrase = statusCodeToReason(code);
}

void	Response::setHeader(const std::string &key, const std::string &value)
{
	this->_headers[key] = value;
}

void	Response::setBody(const std::string &body)
{
	this->_body = body;
	this->_headers["Content-Length"] = numToStr(body.length());
}

/* EXCEPTIONS *************************************************************** */
