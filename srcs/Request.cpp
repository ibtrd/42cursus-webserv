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
	// this->_socket = -42; // DEBUG
};

// Request::Request(const int32_t serverSocket)
// {
// 	this->_socket = accept(serverSocket, NULL, NULL);
// 	if (this->_socket == -1) {
// 		throw std::runtime_error("connection failure");
// 	}
// 	struct epoll_event event;
// 	event.events = EPOLLIN;
// 	event.data.fd = this->_socket;
// 	if (-1 == epoll_ctl(Request::_epollFd, EPOLL_CTL_ADD, this->_socket, &event)) {
// 		close(this->_socket);
// 		throw std::runtime_error(strerror(errno));
// 	}
// 	std::cerr << "Client accepted! fd=" << this->socket() << std::endl;
// 	std::cerr << "BOOL: " << this->_readComplete << std::endl;
// }

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
		ret = this->parseRequestLine();
		if (ret == CONTINUE)
			return (0);
		this->_readComplete = true;

		if (ret == ERROR)
		{
			this->generateResponse(BAD_REQUEST, NULL);
			std::cerr << "400 Bad Request" << std::endl;
		}
		else if (this->_method == INVAL_METHOD)
		{
			this->generateResponse(METHOD_NOT_ALLOWED, NULL);
			std::cerr << "405 Method Not Allowed" << std::endl;
		}
		else if (this->_url != "/")
		{
			this->generateResponse(NOT_FOUND, NULL);
			std::cerr << "404 Not Found" << std::endl;
		}
		else if (this->_url != "/chat")
		{
			std::string body = "Hello, World!aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
			this->generateResponse(OK, &body);
			std::cerr << "200 Chat" << std::endl;
		}
		else if (this->_protocolVersion != "HTTP/1.1")
		{
			this->generateResponse(HTTP_VERSION_NOT_SUPPORTED, NULL);
			std::cerr << "505 HTTP Version Not Supported" << std::endl;
		}
		else
		{
			this->generateResponse(OK, NULL);
			std::cerr << "200 OK" << std::endl;
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

status_t	Request::parseRequestLine(void)
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
	{
		std::cerr << "Invalid request line" << std::endl;
		return (ERROR);
	}
	std::string	method = requestLine.substr(0, pos);
	if (method.empty())
	{
		std::cerr << "Invalid request line" << std::endl;
		return (ERROR);
	}
	this->_method = parseMethod(method);
	requestLine.erase(0, pos + 1);	

	// URL
	pos = requestLine.find(' ');
	if (pos == std::string::npos)
	{
		std::cerr << "Invalid request line" << std::endl;
		return (ERROR);
	}
	this->_url = requestLine.substr(0, pos);
	if (this->_url.empty())
	{
		std::cerr << "Invalid request line" << std::endl;
		return (ERROR);
	}
	requestLine.erase(0, pos + 1);

	// Protocol version
	// pos = requestLine.find_first_of("\010\011\012\013\014 ");	// Check for whitespace (stupid)
	// if (pos != std::string::npos)								//
	// {															//
	// 	std::cerr << "Invalid request line" << std::endl;			//
	// 	return (ERROR);												//
	// }															//
	this->_protocolVersion = requestLine;
	if (this->_protocolVersion.empty())
	{
		std::cerr << "Invalid request line" << std::endl;
		return (ERROR);
	}

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
	bytes = REQ_BUFFER_SIZE > this->_response.response.length() ? this->_response.response.length() : REQ_BUFFER_SIZE;
	bytes = send(this->_socket, this->_response.response.c_str(), bytes, 0);
	if (bytes == -1) {
		std::cerr << "Error: send: " << strerror(errno) << std::endl;
		return (ERROR);
	}
	std::cerr << "Sent: " << bytes << " bytes" << std::endl;
	this->_response.response.erase(0, bytes);
	if (this->_response.response.length())
	{
		return (CONTINUE);
	}
	return (DONE);
}

error_t	Request::generateResponse(const StatusCode code, const std::string *body)
{
	this->_response.status_code = code;
	this->_response.reason_phrase = statusCodeToReason(code);
	this->_response.status_line = "HTTP/1.1 " + numToStr(code) + " " + this->_response.reason_phrase + "\r\n";
	if (body)
	{
		this->_response.headers = "Content-Length: " + numToStr(body->length()) + "\r\n";
		this->_response.body = *body;
	}
	else
	{
		this->_response.headers = "";
		this->_response.body = "";
	}
	this->_response.response = this->_response.status_line + this->_response.headers + "\r\n" + this->_response.body;
	return (0);
}

/* GETTERS ****************************************************************** */

int32_t	Request::socket(void) const { return (this->_socket); }

int32_t	Request::epollFd(void) { return (Request::_epollFd); }

/* SETTERS ****************************************************************** */

void	Request::setEpollFd(const int32_t fd) { Request::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */
