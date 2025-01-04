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
	this->_socket = other._socket;
	this->_buffer = other._buffer;
	this->_method = other._method;
	this->_target = other._target;
	this->_protocolVersion = other._protocolVersion;
	this->_headers = other._headers;
	this->_body = other._body;
	this->_response = other._response;
	this->_readComplete = other._readComplete;
	this->_writeComplete = other._writeComplete;
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
	this->_writeComplete = false;
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

	if (!this->_readComplete)
	{
		// Parse request
		if (this->_protocolVersion.empty()) {
			ret = this->parseRequestLine();
			if (ret == REQ_CONTINUE)
				return (0);
			this->_readComplete = true;
			if (ret == REQ_ERROR)
			{
				std::cerr << "Error something went wrong parsing the request" << std::endl;
				return (1);
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
		// Parse headers
		// ret = this->parseHeaders();
		// Parse body
		
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
		if (ret == REQ_CONTINUE)
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

error_t	Request::parseRequestLine(void)
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
		return (REQ_DONE);
	}
	std::string	method = requestLine.substr(0, pos);
	if (method.empty())
	{
		this->_response.setStatusCode(BAD_REQUEST);
		return (REQ_DONE);
	}
	this->_method = parseMethod(method);
	requestLine.erase(0, pos + 1);
	if (this->_method == INVAL_METHOD)
	{
		this->_response.setStatusCode(METHOD_NOT_ALLOWED);
		return (REQ_DONE);
	}

	// URL
	pos = requestLine.find(' ');
	if (pos == std::string::npos)
	{
		this->_response.setStatusCode(BAD_REQUEST);
		return (REQ_DONE);
	}
	this->_target = requestLine.substr(0, pos);
	if (this->_target.empty())
	{
		this->_response.setStatusCode(BAD_REQUEST);
		return (REQ_DONE);
	}
	requestLine.erase(0, pos + 1);

	this->_protocolVersion = requestLine;
	if (this->_protocolVersion.empty())
	{
		this->_response.setStatusCode(BAD_REQUEST);
		return (REQ_DONE);
	}
	if (this->_protocolVersion != "HTTP/1.1")
	{
		this->_response.setStatusCode(HTTP_VERSION_NOT_SUPPORTED);
		return (REQ_DONE);
	}

	std::cerr << "Method: |" << methodToString(this->_method) << "|" << std::endl;
	std::cerr << "URL: |" << this->_target << "|" << std::endl;
	std::cerr << "Protocol version: |" << this->_protocolVersion << "|" << std::endl;

	return (REQ_DONE);
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

/* GETTERS ****************************************************************** */

int32_t	Request::socket(void) const { return (this->_socket); }

int32_t	Request::epollFd(void) { return (Request::_epollFd); }

/* SETTERS ****************************************************************** */

void	Request::setEpollFd(const int32_t fd) { Request::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */
