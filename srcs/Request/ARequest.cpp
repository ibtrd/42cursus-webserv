#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
# include <dirent.h>

#include "ARequest.hpp"

char	ARequest::_readBuffer[REQ_BUFFER_SIZE];
int32_t	ARequest::_epollFd = -1;

/* CONSTRUCTORS ************************************************************* */

ARequest::ARequest(void)
{
	// std::cerr << "ARequest created" << std::endl;
}

ARequest::ARequest(const ARequest &other)
{
	// std::cerr << "ARequest copy" << std::endl;
	*this = other;
}

ARequest::~ARequest(void)
{
	// std::cerr << "ARequest destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

ARequest	&ARequest::operator=(const ARequest &other)
{
	// std::cerr << "ARequest assign" << std::endl;
	if (this == &other)
		return (*this);
	
	this->_trans = other._trans;

	this->_requestState = other._requestState;

	this->_socket = other._socket;
	this->_buffer = other._buffer;

	this->_method = other._method;
	this->_target = other._target;
	this->_protocolVersion = other._protocolVersion;

	this->_headers = other._headers;

	this->_body = other._body;

	this->_response = other._response;
	this->_responseBuffer = other._responseBuffer;
	return (*this);
}

/* ************************************************************************** */

error_t	ARequest::init(const int32_t requestSocket)
{
	this->_trans = false;
	this->_requestState = REQ_STATE_NONE;
	this->_socket = requestSocket;
	return (0);
}

error_t	ARequest::readSocket(void)
{
	ssize_t	bytes;
	bytes = recv(this->_socket, ARequest::_readBuffer, REQ_BUFFER_SIZE, 0);
	if (bytes == 0)
	{
		std::cerr << "Client disconnected" << std::endl;
		return (1);
	}
	if (bytes == -1)
	{
		std::cerr << "Client error" << std::endl;
		return (1);
	}
	ARequest::_readBuffer[bytes] = '\0';
	std::cerr << "Read:|" << ARequest::_readBuffer << "|" << std::endl;
	this->_buffer += ARequest::_readBuffer;
	ARequest::_readBuffer[0] = '\0';
	return (0);
}

/* ************************************************************************** */

const std::string ARequest::_requestStateStr(void) const
{
	std::string	str("{");

	str += "requestLine: ";
	if (IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_requestState))
		str += "1";
	else
		str += "0";
	str += ", headers: ";
	if (IS_REQ_READ_HEADERS_COMPLETE(this->_requestState))
		str += "1";
	else
		str += "0";
	str += ", body: ";
	if (IS_REQ_READ_BODY_COMPLETE(this->_requestState))
		str += "1";
	else
		str += "0";
	str += ", read: ";
	if (IS_REQ_READ_COMPLETE(this->_requestState))
		str += "1";
	else
		str += "0";
	str += ", writing: ";
	if (IS_REQ_CAN_WRITE(this->_requestState))
		str += "1";
	else
		str += "0";
	str += ", writeComplete: ";
	if (IS_REQ_WRITE_COMPLETE(this->_requestState))
		str += "1";
	else
		str += "0";
	str += "}";
	return (str);
}

/* GETTERS ****************************************************************** */

int32_t	ARequest::socket(void) const { return (this->_socket); }

Method	ARequest::method(void) const { return (this->_method); }

int32_t	ARequest::epollFd(void) { return (ARequest::_epollFd); }

/* SETTERS ****************************************************************** */

void	ARequest::setEpollFd(const int32_t fd) { ARequest::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */
