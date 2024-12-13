#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

#include "Request.hpp"

char	Request::_readBuffer[REQ_BUFFER_SIZE];
int32_t	Request::_epollFd = -1;

/* CONSTRUCTORS ************************************************************* */

Request::Request(void) {};

Request::Request(const int32_t serverSocket) {
	this->_socket = accept(serverSocket, NULL, NULL);
	if (this->_socket == -1) {
		throw std::runtime_error("connection failure");
	}
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = this->_socket;
	if (-1 == epoll_ctl(Request::_epollFd, EPOLL_CTL_ADD, this->_socket, &event)) {
		close(this->_socket);
		throw std::runtime_error(strerror(errno));
	}
	std::cerr << "Client accepted! fd=" << this->socket() << std::endl;
}

Request::Request(const Request &other) {
	*this = other;
}

Request::~Request(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Request	&Request::operator=(const Request &other) {
	if (this == &other)
		return (*this);
	this->_socket = other._socket;
	this->_buffer = other._buffer;
	this->_method = other._method;
	this->_url = other._url;
	this->_protocolVersion = other._protocolVersion;
	this->_headers = other._headers;
	this->_body = other._body;
	return (*this);
}

/* ************************************************************************** */

error_t	Request::handleRequest(void)
{
	error_t	ret = 0;
	ret = this->readSocket();
	if (ret)
		return (ret);
	// std::cerr << "Request: " << this->_buffer << std::endl;

	// Parse request
	if (this->_protocolVersion.empty()) {
		ret = this->parseRequestLine();
		if (ret == CONTINUE)
			return (0);
		if (ret == ERROR)
		{
			// Send 400 Bad Request
			std::cerr << "400 Bad Request" << std::endl;
			return (1);
		}
	}
	return (0);
}

error_t	Request::readSocket(void)
{
	ssize_t	bytes;
	bytes = recv(this->_socket, Request::_readBuffer, REQ_BUFFER_SIZE, 0);
	if (bytes == 0) {
		std::cerr << "Client disconnected" << std::endl;
		return (1);
	}
	if (bytes == -1) {
		std::cerr << "Client error" << std::endl;
		return (1);
	}
	Request::_readBuffer[bytes] = '\0';
	this->_buffer += Request::_readBuffer;
	return (0);
}

status_t	Request::parseRequestLine(void)
{
	std::string	requestLine;

	// Check at least one line is present
	size_t	pos = this->_buffer.find("\r\n");
	if (pos == std::string::npos) {
		std::cerr << "Request too short" << std::endl;
		return (CONTINUE);
	}
	requestLine = this->_buffer.substr(0, pos);
	this->_buffer.erase(0, pos + 2);
	std::cerr << "Request line: |" << requestLine << "|" << std::endl;

	// Parse request line

	// Method
	pos = requestLine.find(' ');
	if (pos == std::string::npos) {
		std::cerr << "Invalid request line" << std::endl;
		return (ERROR);
	}
	std::string	method = requestLine.substr(0, pos);
	if (method.empty()) {
		std::cerr << "Invalid request line" << std::endl;
		return (ERROR);
	}
	this->_method = parseMethod(method);
	requestLine.erase(0, pos + 1);	

	// URL
	pos = requestLine.find(' ');
	if (pos == std::string::npos) {
		std::cerr << "Invalid request line" << std::endl;
		return (ERROR);
	}
	this->_url = requestLine.substr(0, pos);
	if (this->_url.empty()) {
		std::cerr << "Invalid request line" << std::endl;
		return (ERROR);
	}
	requestLine.erase(0, pos + 1);

	// Protocol version
	// pos = requestLine.find_first_of("\010\011\012\013\014 ");	// Check for whitespace (stupid)
	// if (pos != std::string::npos) {								//
	// 	std::cerr << "Invalid request line" << std::endl;			//
	// 	return (ERROR);												//
	// }															//
	this->_protocolVersion = requestLine;
	if (this->_protocolVersion.empty()) {
		std::cerr << "Invalid request line" << std::endl;
		return (ERROR);
	}

	std::cerr << "Method: |" << methodToString(this->_method) << "|" << std::endl;
	std::cerr << "URL: |" << this->_url << "|" << std::endl;
	std::cerr << "Protocol version: |" << this->_protocolVersion << "|" << std::endl;

	return (DONE);
}

error_t	Request::sendResponse(void)
{
	return (0);
}

/* GETTERS ****************************************************************** */

int32_t	Request::socket(void) const { return (this->_socket); }

int32_t	Request::epollFd(void) { return (Request::_epollFd); }

/* SETTERS ****************************************************************** */

void	Request::setEpollFd(const int32_t fd) { Request::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */
