#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
# include <dirent.h>

#include "RequestGET.hpp"
#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

RequestGET::RequestGET(void)
{
	// std::cerr << "RequestGET created" << std::endl;
}

RequestGET::RequestGET(const RequestGET &other)
{
	// std::cerr << "RequestGET copy" << std::endl;
	*this = other;
}

RequestGET::~RequestGET(void)
{
	// std::cerr << "RequestGET destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestGET	&RequestGET::operator=(const RequestGET &other)
{
	std::cerr << "RequestGET assign" << std::endl;
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

ARequest	*RequestGET::clone(void) const
{
	std::cerr << "RequestGET clone" << std::endl;
	return (new RequestGET(*this));
}

error_t	RequestGET::handle(void)
{
	std::cerr << "\nHandling request " << this->_requestStateStr() << std::endl;
	
	error_t	ret = 0;
	/*
	if (!IS_REQ_READ_COMPLETE(this->_requestState) && this->readSocket())
		return (REQ_ERROR);
	// std::cerr << "RequestGET: " << this->_buffer << std::endl;

	if (!IS_REQ_READ_COMPLETE(this->_requestState))
	{
		// Parse body
		// if (!IS_REQ_READ_BODY_COMPLETE(this->_requestState))
		// {
		// 	ret = this->parseBody();
		// 	if (ret == REQ_CONTINUE)
		// 		return (0);
		// 	if (ret == REQ_ERROR)
		// 	{
		// 		std::cerr << "Error something went wrong parsing the body" << std::endl;
		// 		return (1);
		// 	}
		// }

		if (this->_response.statusCode() == NONE)	// DEBUG
		{
			this->_response.setStatusCode(OK);
			this->_response.setBody("GET request");
			/ *	// DEBUG (kécekecé moche 🤮)
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
			* /
		}
	}
	*/

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

	std::cerr << "RequestGET parsed " << this->_requestStateStr() << std::endl;

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
	std::cerr << "RequestGET handled " << this->_requestStateStr() << std::endl;
	return (REQ_DONE);
}

error_t	RequestGET::parseRequest(void)
{
	// Parse body
	// if (!IS_REQ_READ_BODY_COMPLETE(this->_requestState))
	// {
	// 	ret = this->parseBody();
	// 	if (ret == REQ_CONTINUE)
	// 		return (0);
	// 	if (ret == REQ_ERROR)
	// 	{
	// 		std::cerr << "Error something went wrong parsing the body" << std::endl;
	// 		return (1);
	// 	}
	// }

	if (this->_response.statusCode() == NONE)	// DEBUG
	{
		this->_response.setStatusCode(OK);
		this->_response.setBody("GET request");
	}
	return (REQ_DONE);
}

error_t RequestGET::switchToWrite(void)
{
	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = this->_socket;
	if (-1 == epoll_ctl(RequestGET::_epollFd, EPOLL_CTL_MOD, this->_socket, &event))
	{
		close(this->_socket);
		return (-1);
	}
	SET_REQ_CAN_WRITE(this->_requestState);
	return (0);
}

error_t	RequestGET::sendResponse(void)
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

/* OTHERS *********************************************************************/

ARequest	*createRequestGET(void)
{
	return (new RequestGET());
}
