#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

#include "Request.hpp"

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

/* GETTERS ****************************************************************** */

int32_t	Request::socket(void) const { return (this->_socket); }

int32_t	Request::epollFd(void) { return (Request::_epollFd); }

/* SETTERS ****************************************************************** */

void	Request::setEpollFd(const int32_t fd) { Request::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */
