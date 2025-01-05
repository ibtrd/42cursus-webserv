#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
# include <dirent.h>

#include "Request.hpp"
#include "ft.hpp"

char	Request::_readBuffer[REQ_BUFFER_SIZE];
int32_t	Request::_epollFd = -1;

/* CONSTRUCTORS ************************************************************* */

Request::Request(void)
{
	// std::cerr << "Request created" << std::endl;
}

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
	this->_requestState = other._requestState;
	this->_socket = other._socket;
	this->_buffer = other._buffer;
	this->_method = other._method;
	this->_target = other._target;
	this->_protocolVersion = other._protocolVersion;
	this->_headers = other._headers;
	this->_body = other._body;
	this->_response = other._response;
	// this->_readComplete = other._readComplete;
	// this->_writeComplete = other._writeComplete;
	// this->_canWrite = other._canWrite;
	// this->_requestLineComplete = other._requestLineComplete;
	// this->_headersComplete = other._headersComplete;
	// this->_bodyComplete = other._bodyComplete;
	this->_responseBuffer = other._responseBuffer;
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
	this->_requestState = REQ_STATE_NONE;
	// this->_readComplete = false;
	// this->_writeComplete = false;
	// this->_canWrite = false;
	// this->_requestLineComplete = false;
	// this->_headersComplete = false;
	// this->_bodyComplete = false;
	return (0);
}

error_t	Request::handle(void)
{
	std::cerr << "\nHandling request " << this->_requestStateStr() << std::endl;
	
	error_t	ret = 0;
	if (!IS_REQ_READ_COMPLETE(this->_requestState) && this->readSocket())
		return (1);
	// std::cerr << "Request: " << this->_buffer << std::endl;

	if (!IS_REQ_READ_COMPLETE(this->_requestState))
	{
		// Parse request
		if (!IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_requestState)) {
			ret = this->parseRequestLine();
			if (ret == REQ_CONTINUE)
				return (0);
			if (ret == REQ_ERROR)
			{
				std::cerr << "Error something went wrong parsing the request" << std::endl;
				return (1);
			}
			if (this->_response.statusCode() == NONE)	// DEBUG
			{
				// this->_response.setStatusCode(OK);
				// this->_response.setBody("Hello, World!");
				/*	// DEBUG (kécekecé moche 🤮)
				// if (this->_method == GET)
				// {
				// 	std::string path = "/home/kchillon/42cursus/42cursus-webserv/" + this->_target;
				// 	if (access(path.c_str(), F_OK) == 0)
				// 	{
				// 		if (access(path.c_str(), R_OK) == 0)
				// 		{
				// 			if (ft::is_dir((char *)path.c_str()))
				// 			{
				// 				// this->_response.setBody("GET request on a directory");
				// 				DIR *dir;

				// 				dir = opendir(path.c_str());
				// 				if (!dir)
				// 				{
				// 					this->_response.setStatusCode(INTERNAL_SERVER_ERROR);
				// 				}
				// 				else
				// 				{
				// 					std::string body;

				// 					body = "<html><head><title>Index of " + this->_target + "</title></head><body><h1>Index of " + this->_target + "</h1><hr><pre>";

				// 					struct dirent *entry;
				// 					while ((entry = readdir(dir)) != NULL)
				// 					{
				// 						body += "<a href=\"http://127.0.0.1:8080" + this->_target + "/" + entry->d_name + "\">" + entry->d_name + "</a><br>";
				// 					}
				// 					body += "</pre><hr></body></html>";
				// 					this->_response.setBody(body);
				// 					closedir(dir);
				// 				}
				// 			}
				// 			else
				// 			{
				// 				this->_response.setBody("GET request on a file");
				// 			}
				// 		}
				// 		else
				// 		{
				// 			this->_response.setStatusCode(FORBIDDEN);
				// 		}
				// 	}
				// 	else
				// 	{
				// 		this->_response.setStatusCode(NOT_FOUND);
				// 	}
				// }
				// else if (this->_method == POST)
				// {
				// 	this->_response.setBody("POST request");
				// }
				// else if (this->_method == DELETE)
				// {
				// 	this->_response.setBody("DELETE request");
				// }
				*/
			}
			else
			{
				std::cout << "Response status code: " << this->_response.statusCode() << std::endl;
			}
		}
		// Parse headers
		if (!IS_REQ_READ_HEADERS_COMPLETE(this->_requestState))
		{
			ret = this->parseHeaders();
			if (ret == REQ_CONTINUE)
				return (0);
			if (ret == REQ_ERROR)
			{
				std::cerr << "Error something went wrong parsing the headers" << std::endl;
				return (1);
			}
		}
		// Parse body
		if (!IS_REQ_READ_BODY_COMPLETE(this->_requestState))
		{
			ret = this->parseBody();
			if (ret == REQ_CONTINUE)
				return (0);
			if (ret == REQ_ERROR)
			{
				std::cerr << "Error something went wrong parsing the body" << std::endl;
				return (1);
			}
		}
		

		if (this->_response.statusCode() == NONE)	// DEBUG
		{
			this->_response.setStatusCode(OK);
			this->_response.setBody("Hello, World!");
			/*	// DEBUG (kécekecé moche 🤮)
			// if (this->_method == GET)
			// {
			// 	std::string path = "/home/kchillon/42cursus/42cursus-webserv/" + this->_target;
			// 	if (access(path.c_str(), F_OK) == 0)
			// 	{
			// 		if (access(path.c_str(), R_OK) == 0)
			// 		{
			// 			if (ft::is_dir((char *)path.c_str()))
			// 			{
			// 				// this->_response.setBody("GET request on a directory");
			// 				DIR *dir;

			// 				dir = opendir(path.c_str());
			// 				if (!dir)
			// 				{
			// 					this->_response.setStatusCode(INTERNAL_SERVER_ERROR);
			// 				}
			// 				else
			// 				{
			// 					std::string body;

			// 					body = "<html><head><title>Index of " + this->_target + "</title></head><body><h1>Index of " + this->_target + "</h1><hr><pre>";

			// 					struct dirent *entry;
			// 					while ((entry = readdir(dir)) != NULL)
			// 					{
			// 						body += "<a href=\"http://127.0.0.1:8080" + this->_target + "/" + entry->d_name + "\">" + entry->d_name + "</a><br>";
			// 					}
			// 					body += "</pre><hr></body></html>";
			// 					this->_response.setBody(body);
			// 					closedir(dir);
			// 				}
			// 			}
			// 			else
			// 			{
			// 				this->_response.setBody("GET request on a file");
			// 			}
			// 		}
			// 		else
			// 		{
			// 			this->_response.setStatusCode(FORBIDDEN);
			// 		}
			// 	}
			// 	else
			// 	{
			// 		this->_response.setStatusCode(NOT_FOUND);
			// 	}
			// }
			// else if (this->_method == POST)
			// {
			// 	this->_response.setBody("POST request");
			// }
			// else if (this->_method == DELETE)
			// {
			// 	this->_response.setBody("DELETE request");
			// }
			*/
		}
	}

	std::cerr << "Request parsed " << this->_requestStateStr() << std::endl;

	// Switch to write mode
	if (!IS_REQ_CAN_WRITE(this->_requestState))
	{
		if (this->switchToWrite())
		{
			std::cerr << "Error: epoll_ctl: " << strerror(errno) << std::endl;
			return (1);
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
			usleep(500000); // DEBUG
			return (0);
		}
		std::cerr << "Done responding" << std::endl;
	}
	std::cerr << "Request handled " << this->_requestStateStr() << std::endl;
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

error_t	Request::parseRequestLine(void)	// HEAVY rework needed
{
	std::string	requestLine;

	// Check at least one line is present
	size_t	pos = this->_buffer.find("\r\n");
	if (pos == std::string::npos)
	{
		std::cerr << "Request too short" << std::endl;
		return (REQ_CONTINUE);
	}
	requestLine = this->_buffer.substr(0, pos);
	this->_buffer.erase(0, pos + 2);
	std::cerr << "Request line: |" << requestLine << "|" << std::endl;

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
	this->_method = parseMethod(method);
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
	std::cerr << "URL: |" << this->_target << "|" << std::endl;
	std::cerr << "Protocol version: |" << this->_protocolVersion << "|" << std::endl;

	SET_REQ_READ_REQUEST_LINE_COMPLETE(this->_requestState);

	return (REQ_DONE);
}

error_t	Request::parseHeaders(void)
{
	size_t	pos;
	std::string	line;
	std::string	key;
	std::string	value;

	while ((pos = this->_buffer.find("\r\n")) != std::string::npos)
	{
		line = this->_buffer.substr(0, pos);
		this->_buffer.erase(0, pos + 2);
		if (line.empty())
		{
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
	SET_REQ_CAN_WRITE(this->_requestState);
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

// error_t

/* ************************************************************************** */

const std::string Request::_requestStateStr(void) const
{
	std::string	str("{");

	str += "requestLine: ";
	if (IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_requestState))
		str += "true";
	else
		str += "false";
	str += ", headers: ";
	if (IS_REQ_READ_HEADERS_COMPLETE(this->_requestState))
		str += "true";
	else
		str += "false";
	str += ", body: ";
	if (IS_REQ_READ_BODY_COMPLETE(this->_requestState))
		str += "true";
	else
		str += "false";
	str += ", read: ";
	if (IS_REQ_READ_COMPLETE(this->_requestState))
		str += "true";
	else
		str += "false";
	str += ", writing: ";
	if (IS_REQ_CAN_WRITE(this->_requestState))
		str += "true";
	else
		str += "false";
	str += ", writeComplete: ";
	if (IS_REQ_WRITE_COMPLETE(this->_requestState))
		str += "true";
	else
		str += "false";
	str += "}";
	return (str);
}

/* GETTERS ****************************************************************** */

int32_t	Request::socket(void) const { return (this->_socket); }

int32_t	Request::epollFd(void) { return (Request::_epollFd); }

/* SETTERS ****************************************************************** */

void	Request::setEpollFd(const int32_t fd) { Request::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */
