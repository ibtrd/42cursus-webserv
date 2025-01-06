#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
# include <dirent.h>

#include "RequestMaster.hpp"
#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

RequestMaster::RequestMaster(void)
{
	// std::cerr << "RequestMaster created" << std::endl;
}

RequestMaster::RequestMaster(const RequestMaster &other)
{
	// std::cerr << "RequestMaster copy" << std::endl;
	*this = other;
}

RequestMaster::~RequestMaster(void)
{
	// std::cerr << "RequestMaster destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestMaster	&RequestMaster::operator=(const RequestMaster &other)
{
	std::cerr << "RequestMaster assign" << std::endl;
	(void)other;
	// if (this == &other)
	// 	return (*this);

	// this->_trans = other._trans;

	// this->_requestState = other._requestState;

	// this->_socket = other._socket;
	// this->_buffer = other._buffer;
	// this->_skipNextRead = other._skipNextRead;

	// this->_method = other._method;
	// this->_target = other._target;
	// this->_protocolVersion = other._protocolVersion;

	// this->_headers = other._headers;

	// this->_body = other._body;

	// this->_response = other._response;
	// this->_responseBuffer = other._responseBuffer;
	return (*this);
}

/* ************************************************************************** */

error_t RequestMaster::init(const int32_t requestSocket)
{
	this->_socket = requestSocket;
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = this->_socket;
	if (-1 == epoll_ctl(RequestMaster::_epollFd, EPOLL_CTL_ADD, this->_socket, &event)) {
		close(this->_socket);
		return (-1);
	}
	std::cerr << "Client accepted! fd=" << this->socket() << std::endl;
	this->_trans = false;
	this->_requestState = REQ_STATE_NONE;
	this->_skipNextRead = false;
	return (0);
}

error_t	RequestMaster::handle(void)
{
	std::cerr << "\nHandling request " << this->_requestStateStr() << std::endl;
	
	error_t	ret = 0;
	// if (!IS_REQ_READ_COMPLETE(this->_requestState) && this->readSocket())
	// 	return (REQ_ERROR);
	// // std::cerr << "RequestMaster: " << this->_buffer << std::endl;

	// if (!IS_REQ_READ_COMPLETE(this->_requestState))
	// {
	// 	// Parse request
	// 	if (!IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_requestState)) {
	// 		ret = this->parseRequestLine();
	// 		if (ret == REQ_CONTINUE)
	// 			return (REQ_CONTINUE);
	// 		if (ret == REQ_ERROR)
	// 		{
	// 			std::cerr << "Error something went wrong parsing the request" << std::endl;
	// 			return (REQ_ERROR);
	// 		}
	// 		// if (this->_response.statusCode() != NONE)	// DEBUG
	// 		// {
	// 		// 	std::cout << "Response status code: " << this->_response.statusCode() << std::endl;
	// 		// }
	// 	}
	// 	// Parse headers
	// 	if (!IS_REQ_READ_HEADERS_COMPLETE(this->_requestState))
	// 	{
	// 		ret = this->parseHeaders();
	// 		if (ret == REQ_CONTINUE)
	// 			return (REQ_CONTINUE);
	// 		if (ret == REQ_ERROR)
	// 		{
	// 			std::cerr << "Error something went wrong parsing the headers" << std::endl;
	// 			return (REQ_ERROR);
	// 		}
	// 	}

	// 	if (this->_response.statusCode() == NONE)	// If no error occured until now transfer to specialised request handler
	// 	{
	// 		std::cerr << "Transfer to specialised request handler" << std::endl;
	// 		return (REQ_TRANSFER);
	// 	}
	// }

	if (!IS_REQ_READ_COMPLETE(this->_requestState))
	{
		if (!this->_skipNextRead && (ret = this->readSocket()) != REQ_CONTINUE)
			return (ret);
		this->_skipNextRead = false;

		switch (this->parseRequest())
		{
		case REQ_CONTINUE:
			return (REQ_CONTINUE);

		case REQ_ERROR:
			std::cerr << "Error something went wrong parsing the request" << std::endl;
			return (REQ_ERROR);
		
		case REQ_TRANSFER:
			std::cerr << "Transfer to specialised request handler" << std::endl;
			this->_skipNextRead = true;
			return (REQ_TRANSFER);
		
		default:
			break;
		}
	}

	std::cerr << "RequestMaster parsed " << this->_requestStateStr() << std::endl;

	// Switch to write mode
	if (!IS_REQ_CAN_WRITE(this->_requestState))
	{
		if (this->switchToWrite())
		{
			std::cerr << "Error: epoll_ctl: " << strerror(errno) << std::endl;
			return (REQ_ERROR);
		}
		this->_responseBuffer = this->_response.response();
	}

	std::cerr << "Switched to write " << this->_requestStateStr() << std::endl;

	// Send response
	if (IS_REQ_CAN_WRITE(this->_requestState))
	{
		ret = this->sendResponse();
		if (ret == REQ_CONTINUE)
		{
			// usleep(500000); // DEBUG
			return (REQ_CONTINUE);
		}
		std::cerr << "Done responding" << std::endl;
	}
	std::cerr << "RequestMaster handled " << this->_requestStateStr() << std::endl;
	return (REQ_DONE);
}

error_t RequestMaster::parseRequest(void)
{
	error_t	ret;

	// Parse request line
	if (!IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_requestState)) {
		ret = this->parseRequestLine();
		if (ret != REQ_DONE)
			return (ret);
	}

	// Parse headers
	if (!IS_REQ_READ_HEADERS_COMPLETE(this->_requestState))
	{
		ret = this->parseHeaders();
		if (ret != REQ_DONE)
			return (ret);
	}

	// If no error occured until now transfer to specialised request handler
	if (this->_response.statusCode() == NONE)
	{
		std::cerr << "Transfer to specialised request handler" << std::endl;
		return (REQ_TRANSFER);
	}
	return (REQ_DONE);
}

error_t	RequestMaster::parseRequestLine(void)	// HEAVY rework needed
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

	// URL
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

error_t	RequestMaster::parseHeaders(void)
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

error_t RequestMaster::switchToWrite(void)
{
	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = this->_socket;
	if (-1 == epoll_ctl(RequestMaster::_epollFd, EPOLL_CTL_MOD, this->_socket, &event))
	{
		close(this->_socket);
		return (-1);
	}
	SET_REQ_CAN_WRITE(this->_requestState);
	return (0);
}

error_t	RequestMaster::sendResponse(void)
{
	std::cerr << "Sending response..." << std::endl;
	ssize_t	bytes;
	bytes = REQ_BUFFER_SIZE > this->_responseBuffer.length() ? this->_responseBuffer.length() : REQ_BUFFER_SIZE;
	bytes = send(this->_socket, this->_responseBuffer.c_str(), bytes, 0);
	if (bytes == -1) {
		std::cerr << "Error: send: " << strerror(errno) << std::endl;
		return (REQ_ERROR);
	}
	std::cerr << "Sent: " << bytes << " bytes" << std::endl;
	this->_responseBuffer.erase(0, bytes);
	if (this->_responseBuffer.length())
	{
		return (REQ_CONTINUE);
	}
	return (REQ_DONE);
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */
