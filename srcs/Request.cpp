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

Request::Request(void)
{
	// std::cerr << "Request created" << std::endl;
};

Request::Request(const Request &other)
{
	// std::cerr << "Request copy" << std::endl;
	*this = other;
}

Request::~Request(void)
{
	// std::cerr << "Request destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

Request	&Request::operator=(const Request &other)
{
	// std::cerr << "Request assign" << std::endl;
	if (this == &other)
		return (*this);
	this->_socket = other._socket;
	this->_buffer = other._buffer;
	this->_method = other._method;
	this->_url = other._url;
	this->_protocolVersion = other._protocolVersion;
	this->_headers = other._headers;
	this->_body = other._body;
	this->_response = other._response;
	this->_readComplete = other._readComplete;
	this->_canWrite = other._canWrite;
	return (*this);
}

/* ************************************************************************** */

error_t Request::init(const int32_t requestSocket)
{
	this->_socket = requestSocket;
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = this->_socket;
	if (-1 == epoll_ctl(Request::_epollFd, EPOLL_CTL_ADD, this->_socket, &event)) {
		close(this->_socket);
		return (-1);
	}
	std::cerr << "Client accepted! fd=" << this->socket() << std::endl;
	this->_readComplete = false;
	this->_canWrite = false;
	return (0);
}

error_t	Request::handle(void)
{
	std::cerr << "\nHandling request " << this->_readComplete << this->_protocolVersion.empty() << std::endl;
	error_t	ret = 0;
	if (!this->_readComplete && this->readSocket())
		return (1);
	// std::cerr << "Request: " << this->_buffer << std::endl;

	// Parse request
	if (this->_protocolVersion.empty()) {
		StatusCode parseStatus = this->parseRequestLine();
		if (parseStatus == CONTINUE)
			return (0);
		this->_readComplete = true;
		if (parseStatus == ERROR)
		{
			std::cerr << "Error something went wrong parsing the request" << std::endl;
			return (1);
		}
		if (parseStatus != DONE)
			this->_response.setStatusCode(parseStatus);
		else	// DEBUG
		{
			this->_response.setStatusCode(OK);
			this->_response.setBody("Hello, World!");
		}
	}

	// Switch to write mode
	if (this->_readComplete && !this->_canWrite)
	{
		if (this->switchToWrite())
		{
			std::cerr << "Error: epoll_ctl: " << strerror(errno) << std::endl;
			return (1);
		}
		this->_responseBuffer = this->_response.response();
	}

	// Send response
	if (this->_canWrite)
	{
		ret = this->sendResponse();
		if (ret == CONTINUE)
		{
			usleep(500000); // DEBUG
			return (0);
		}
		std::cerr << "Done responding" << std::endl;
	}
	return (1);
}

error_t	Request::readSocket(void)
{
	ssize_t	bytes;
	bytes = recv(this->_socket, Request::_readBuffer, REQ_BUFFER_SIZE, 0);
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
	Request::_readBuffer[bytes] = '\0';
	std::cerr << "Read:|" << Request::_readBuffer << "|" << std::endl;
	this->_buffer += Request::_readBuffer;
	Request::_readBuffer[0] = '\0';
	return (0);
}

StatusCode	Request::parseRequestLine(void)
{
	std::string	requestLine;

	// Check at least one line is present
	size_t	pos = this->_buffer.find("\r\n");
	if (pos == std::string::npos)
	{
		std::cerr << "Request too short" << std::endl;
		return (CONTINUE);
	}
	requestLine = this->_buffer.substr(0, pos);
	this->_buffer.erase(0, pos + 2);
	std::cerr << "Request line: |" << requestLine << "|" << std::endl;

	// Parse request line

	// Method
	pos = requestLine.find(' ');
	if (pos == std::string::npos)
		return (BAD_REQUEST);
	std::string	method = requestLine.substr(0, pos);
	if (method.empty())
		return (BAD_REQUEST);
	this->_method = parseMethod(method);
	requestLine.erase(0, pos + 1);
	if (this->_method == INVAL_METHOD)
		return (METHOD_NOT_ALLOWED);

	// URL
	pos = requestLine.find(' ');
	if (pos == std::string::npos)
		return (BAD_REQUEST);
	this->_url = requestLine.substr(0, pos);
	if (this->_url.empty())
		return (BAD_REQUEST);
	requestLine.erase(0, pos + 1);

	this->_protocolVersion = requestLine;
	if (this->_protocolVersion.empty())
		return (BAD_REQUEST);
	if (this->_protocolVersion != "HTTP/1.1")
		return (HTTP_VERSION_NOT_SUPPORTED);

	std::cerr << "Method: |" << methodToString(this->_method) << "|" << std::endl;
	std::cerr << "URL: |" << this->_url << "|" << std::endl;
	std::cerr << "Protocol version: |" << this->_protocolVersion << "|" << std::endl;

	return (DONE);
}

error_t Request::switchToWrite(void)
{
	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = this->_socket;
	if (-1 == epoll_ctl(Request::_epollFd, EPOLL_CTL_MOD, this->_socket, &event))
	{
		close(this->_socket);
		return (-1);
	}
	this->_canWrite = true;
	return (0);
}

error_t	Request::sendResponse(void)
{
	std::cerr << "Sending response..." << std::endl;
	ssize_t	bytes;
	bytes = REQ_BUFFER_SIZE > this->_responseBuffer.length() ? this->_responseBuffer.length() : REQ_BUFFER_SIZE;
	bytes = send(this->_socket, this->_responseBuffer.c_str(), bytes, 0);
	if (bytes == -1) {
		std::cerr << "Error: send: " << strerror(errno) << std::endl;
		return (ERROR);
	}
	std::cerr << "Sent: " << bytes << " bytes" << std::endl;
	this->_responseBuffer.erase(0, bytes);
	if (this->_responseBuffer.length())
	{
		return (CONTINUE);
	}
	return (DONE);
}

/* GETTERS ****************************************************************** */

int32_t	Request::socket(void) const { return (this->_socket); }

int32_t	Request::epollFd(void) { return (Request::_epollFd); }

/* SETTERS ****************************************************************** */

void	Request::setEpollFd(const int32_t fd) { Request::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */
