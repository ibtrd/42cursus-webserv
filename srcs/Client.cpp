#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
# include <dirent.h>

#include "Client.hpp"
#include "RequestGET.hpp"

char	Client::_readBuffer[REQ_BUFFER_SIZE];
int32_t	Client::_epollFd = -1;
ARequest	*(*Client::_requestsBuilder[INVAL_METHOD])(void) = {
	createRequestGET,
	NULL,
	NULL
};

/* CONSTRUCTORS ************************************************************* */

Client::Client(void)
{
	// std::cerr << "Client created" << std::endl;
	this->_request = NULL;
}

Client::Client(const Client &other)
{
	// std::cerr << "Client copy" << std::endl;
	this->_request = NULL;
	*this = other;
}

Client::~Client(void)
{
	// std::cerr << "Client destroyed" << std::endl;
	if (this->_request)
		delete this->_request;
}

/* OPERATOR OVERLOADS ******************************************************* */

Client	&Client::operator=(const Client &other)
{
	std::cerr << "Client assign" << std::endl;
	if (this == &other)
		return (*this);

	// this->_request = other._request;
	if (this->_request)
		delete this->_request;
	if (other._request)
		this->_request = other._request->clone();
	else
		this->_request = NULL;
	this->_requestState = other._requestState;

	this->_socket = other._socket;
	this->_buffer = other._buffer;

	this->_method = other._method;
	this->_target = other._target;
	this->_protocolVersion = other._protocolVersion;

	this->_headers = other._headers;

	// this->_body = other._body;

	this->_response = other._response;
	this->_responseBuffer = other._responseBuffer;
	return (*this);
}

/* ************************************************************************** */

const std::string Client::_requestStateStr(void) const
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

error_t	Client::_readSocket(void)
{
	ssize_t	bytes;
	bytes = recv(this->_socket, Client::_readBuffer, REQ_BUFFER_SIZE, 0);
	if (bytes == 0)
	{
		std::cerr << "Client disconnected" << std::endl;
		return (REQ_DONE);
	}
	if (bytes == -1)
	{
		std::cerr << "Client error" << std::endl;
		return (REQ_ERROR);
	}
	Client::_readBuffer[bytes] = '\0';
	std::cerr << "Read:|" << Client::_readBuffer << "|" << std::endl;
	this->_buffer += Client::_readBuffer;
	Client::_readBuffer[0] = '\0';
	return (REQ_CONTINUE);
}

error_t Client::_parseRequest(void)
{
	error_t ret;

	// Parse request line
	if (!IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_requestState)) {
		ret = this->_parseRequestLine();
		if (ret != REQ_DONE)
			return (ret);
	}

	// Parse headers
	if (!IS_REQ_READ_HEADERS_COMPLETE(this->_requestState))
	{
		ret = this->_parseHeaders();
		if (ret != REQ_DONE)
			return (ret);
	}

	SET_REQ_CLIENT_READ_COMPLETE(this->_requestState);
	if (this->_response.statusCode() != NONE)
		SET_REQ_READ_COMPLETE(this->_requestState);
	else
		this->_request = Client::_requestsBuilder[this->_method]();
	return (REQ_DONE);
}

error_t Client::_parseRequestLine(void)
{
	std::string	requestLine;

	// Check at least one line is present
	size_t	pos = this->_buffer.find("\r\n");
	if (pos == std::string::npos)
	{
		std::cerr << "RequestMaster too short" << std::endl;
		return (REQ_CONTINUE);
	}
	requestLine = this->_buffer.substr(0, pos);
	this->_buffer.erase(0, pos + 2);
	std::cerr << "RequestMaster line: |" << requestLine << "|" << std::endl;

	// Parse request line

	// Method
	pos = requestLine.find(' ');
	if (pos == std::string::npos)
	{
		this->_response.setStatusCode(BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_requestState);
		return (REQ_DONE);
	}
	std::string	method = requestLine.substr(0, pos);
	if (method.empty())
	{
		this->_response.setStatusCode(BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_requestState);
		return (REQ_DONE);
	}
	this->_method = stringToMethod(method);
	requestLine.erase(0, pos + 1);
	if (this->_method == INVAL_METHOD)
	{
		this->_response.setStatusCode(METHOD_NOT_ALLOWED);
		SET_REQ_READ_COMPLETE(this->_requestState);
		return (REQ_DONE);
	}

	// Target
	pos = requestLine.find(' ');
	if (pos == std::string::npos)
	{
		this->_response.setStatusCode(BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_requestState);
		return (REQ_DONE);
	}
	this->_target = requestLine.substr(0, pos);
	if (this->_target.empty())
	{
		this->_response.setStatusCode(BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_requestState);
		return (REQ_DONE);
	}
	requestLine.erase(0, pos + 1);

	// Protocol version
	this->_protocolVersion = requestLine;
	if (this->_protocolVersion.empty())
	{
		this->_response.setStatusCode(BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_requestState);
		return (REQ_DONE);
	}
	if (this->_protocolVersion != "HTTP/1.1")
	{
		this->_response.setStatusCode(HTTP_VERSION_NOT_SUPPORTED);
		SET_REQ_READ_COMPLETE(this->_requestState);
		return (REQ_DONE);
	}

	std::cerr << "Method: |" << methodToString(this->_method) << "|" << std::endl;
	std::cerr << "Target: |" << this->_target << "|" << std::endl;
	std::cerr << "Protocol version: |" << this->_protocolVersion << "|" << std::endl;

	SET_REQ_READ_REQUEST_LINE_COMPLETE(this->_requestState);

	return (REQ_DONE);
}

error_t Client::_parseHeaders(void)
{
	size_t	pos;
	std::string	line;
	std::string	key;
	std::string	value;

	std::cerr << "Parsing headers..." << std::endl;
	while ((pos = this->_buffer.find("\r\n")) != std::string::npos)
	{
		line = this->_buffer.substr(0, pos);
		this->_buffer.erase(0, pos + 2);
		if (line.empty())
		{
			if (this->_headers.find("Host") == this->_headers.end())
			{
				this->_response.setStatusCode(BAD_REQUEST);
				SET_REQ_READ_COMPLETE(this->_requestState);
			}
			else
				SET_REQ_READ_HEADERS_COMPLETE(this->_requestState);
			return (REQ_DONE);
		}
		pos = line.find(": ");
		if (pos == std::string::npos)
		{
			this->_response.setStatusCode(BAD_REQUEST);
			SET_REQ_READ_COMPLETE(this->_requestState);
			return (REQ_DONE);
		}
		key = line.substr(0, pos);
		value = line.substr(pos + 2);
		this->_headers[key] = value;
		std::cerr << "Header: |" << key << "| |" << value << "|" << std::endl;
	}
	return (REQ_CONTINUE);
}

/* ************************************************************************** */

error_t	Client::init(const int32_t requestSocket)
{
	this->_socket = requestSocket;
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = this->_socket;
	if (-1 == epoll_ctl(Client::_epollFd, EPOLL_CTL_ADD, this->_socket, &event)) {
		close(this->_socket);
		return (-1);
	}
	std::cerr << "Client accepted! fd=" << this->_socket << std::endl;
	this->_requestState = REQ_STATE_NONE;
	return (0);
}

error_t	Client::handle(void)
{
	error_t	ret;

	if (!IS_REQ_READ_COMPLETE(this->_requestState)
		&& (ret = this->_readSocket()) != REQ_CONTINUE)
		return (ret);
	
	if (!IS_REQ_CLIENT_READ_COMPLETE(this->_requestState)
		&& (ret = this->_parseRequest()) != REQ_DONE)
		return (ret);
	
	/* stuff */

	return (REQ_DONE);	// END
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

// void	Client::skipNextRead(void) { this->_skipNextRead = true; }

void	Client::setEpollFd(const int32_t fd) { Client::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */
