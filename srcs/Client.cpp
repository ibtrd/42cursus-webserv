#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
# include <dirent.h>

#include "Client.hpp"
#include "RequestGET.hpp"
#include "RequestPOST.hpp"
#include "RequestDELETE.hpp"

char	Client::_readBuffer[REQ_BUFFER_SIZE];
int32_t	Client::_epollFd = -1;
ARequest	*(*Client::_requestsBuilder[INVAL_METHOD])(RequestContext_t &) = {
	createRequestGET,
	createRequestPOST,
	createRequestDELETE
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

	if (this->_request)
		delete this->_request;
	if (other._request)
		this->_request = other._request->clone();
	else
		this->_request = NULL;

	this->_socket = other._socket;
	this->_context = other._context;
	return (*this);
}

/* ************************************************************************** */

const std::string Client::_requestStateStr(void) const
{
	std::string	str("{");

	str += "requestLine: ";
	if (IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_context.requestState))
		str += "1";
	else
		str += "0";
	str += ", headers: ";
	if (IS_REQ_READ_HEADERS_COMPLETE(this->_context.requestState))
		str += "1";
	else
		str += "0";
	str += ", body: ";
	if (IS_REQ_READ_BODY_COMPLETE(this->_context.requestState))
		str += "1";
	else
		str += "0";
	str += ", clientRead: ";
	if (IS_REQ_CLIENT_READ_COMPLETE(this->_context.requestState))
		str += "1";
	else
		str += "0";
	str += ", read: ";
	if (IS_REQ_READ_COMPLETE(this->_context.requestState))
		str += "1";
	else
		str += "0";
	str += ", process: ";
	if (IS_REQ_PROCESS_COMPLETE(this->_context.requestState))
		str += "1";
	else
		str += "0";
	str += ", writing: ";
	if (IS_REQ_CAN_WRITE(this->_context.requestState))
		str += "1";
	else
		str += "0";
	str += ", writeComplete: ";
	if (IS_REQ_WRITE_COMPLETE(this->_context.requestState))
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
	this->_context.buffer += Client::_readBuffer;
	Client::_readBuffer[0] = '\0';
	return (REQ_CONTINUE);
}

error_t Client::_parseRequest(void)
{
	error_t ret;

	// Parse request line
	if (!IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_context.requestState)) {
		ret = this->_parseRequestLine();
		if (ret != REQ_DONE)
			return (ret);
	}

	// Parse headers
	if (!IS_REQ_READ_HEADERS_COMPLETE(this->_context.requestState))
	{
		ret = this->_parseHeaders();
		if (ret != REQ_DONE)
			return (ret);
	}

	SET_REQ_CLIENT_READ_COMPLETE(this->_context.requestState);

	// Search for a rule block if no response has been set
	if (this->_context.response.statusCode() == NONE) {
		// Find rule block
		this->_context.ruleBlock = (void *)this->findServerBlock(this->_serverSocket, this->_context.headers["Host"]);
		this->_context.ruleBlock = (void *)((ServerBlock *)this->_context.ruleBlock)->findLocationBlock(this->_context.target);
		if (this->_context.ruleBlock) {
			std::cerr << "RuleBlock: " << *((LocationBlock *)this->_context.ruleBlock) << std::endl;
			this->_request = Client::_requestsBuilder[this->_context.method](this->_context);
			return (REQ_DONE);
		}
		std::cerr << "No rule block found" << std::endl;
		this->_context.response.setStatusCode(NOT_FOUND);
	}

	SET_REQ_READ_COMPLETE(this->_context.requestState); // probably not needed
	SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
	this->_switchToWrite();
	this->_context.responseBuffer = this->_context.response.response();
	return (REQ_DONE);
}

error_t Client::_parseRequestLine(void)
{
	std::string	requestLine;

	// Check at least one line is present
	size_t	pos = this->_context.buffer.find("\r\n");
	if (pos == std::string::npos)
	{
		std::cerr << "RequestLine too short" << std::endl;
		return (REQ_CONTINUE);
	}
	requestLine = this->_context.buffer.substr(0, pos);
	this->_context.buffer.erase(0, pos + 2);
	std::cerr << "RequestLine line: |" << requestLine << "|" << std::endl;

	// Parse request line

	// Method
	pos = requestLine.find(' ');
	if (pos == std::string::npos)
	{
		this->_context.response.setStatusCode(BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	std::string	method = requestLine.substr(0, pos);
	if (method.empty())
	{
		this->_context.response.setStatusCode(BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	this->_context.method = stringToMethod(method);
	requestLine.erase(0, pos + 1);
	if (this->_context.method == INVAL_METHOD)
	{
		this->_context.response.setStatusCode(METHOD_NOT_ALLOWED);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	// Target
	pos = requestLine.find(' ');
	if (pos == std::string::npos)
	{
		this->_context.response.setStatusCode(BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	this->_context.target = requestLine.substr(0, pos);
	if (this->_context.target.empty() || this->_context.target[0] != '/')
	{
		this->_context.response.setStatusCode(BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	requestLine.erase(0, pos + 1);

	// Protocol version
	this->_context.protocolVersion = requestLine;
	if (this->_context.protocolVersion.empty())
	{
		this->_context.response.setStatusCode(BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (this->_context.protocolVersion != "HTTP/1.1")
	{
		this->_context.response.setStatusCode(HTTP_VERSION_NOT_SUPPORTED);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	std::cerr << "Method: |" << methodToString(this->_context.method) << "|" << std::endl;
	std::cerr << "Target: |" << this->_context.target << "|" << std::endl;
	std::cerr << "Protocol version: |" << this->_context.protocolVersion << "|" << std::endl;

	SET_REQ_READ_REQUEST_LINE_COMPLETE(this->_context.requestState);

	return (REQ_DONE);
}

error_t Client::_parseHeaders(void)
{
	size_t	pos;
	std::string	line;
	std::string	key;
	std::string	value;

	std::cerr << "Parsing headers..." << std::endl;
	while ((pos = this->_context.buffer.find("\r\n")) != std::string::npos)
	{
		line = this->_context.buffer.substr(0, pos);
		this->_context.buffer.erase(0, pos + 2);
		if (line.empty())
		{
			if (this->_context.headers.find("Host") == this->_context.headers.end())
			{
				this->_context.response.setStatusCode(BAD_REQUEST);
				SET_REQ_READ_COMPLETE(this->_context.requestState);
			}
			else
				SET_REQ_READ_HEADERS_COMPLETE(this->_context.requestState);
			return (REQ_DONE);
		}
		pos = line.find(": ");
		if (pos == std::string::npos)
		{
			this->_context.response.setStatusCode(BAD_REQUEST);
			SET_REQ_READ_COMPLETE(this->_context.requestState);
			return (REQ_DONE);
		}
		key = line.substr(0, pos);
		value = line.substr(pos + 2);
		this->_context.headers[key] = value;
		std::cerr << "Header: |" << key << "| |" << value << "|" << std::endl;
	}
	return (REQ_CONTINUE);
}

error_t	Client::_process(void)
{
	error_t	ret;

	ret = this->_request->parse();
	if (ret != REQ_DONE)
		return (ret);

	ret = this->_request->process();
	if (ret != REQ_ERROR)
		return (ret);

	return (REQ_DONE);
}

error_t Client::_switchToWrite(void)
{
	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = this->_socket;
	if (-1 == epoll_ctl(Client::_epollFd, EPOLL_CTL_MOD, this->_socket, &event))
	{
		close(this->_socket);
		return (-1);
	}
	SET_REQ_CAN_WRITE(this->_context.requestState);
	return (0);
}

error_t	Client::_sendResponse(void)
{
	std::cerr << "Sending response..." << std::endl;
	ssize_t	bytes;

	bytes = REQ_BUFFER_SIZE > this->_context.responseBuffer.length() ? this->_context.responseBuffer.length() : REQ_BUFFER_SIZE;
	bytes = send(this->_socket, this->_context.responseBuffer.c_str(), bytes, 0);
	if (bytes == -1) {
		std::cerr << "Error: send: " << strerror(errno) << std::endl;
		return (REQ_ERROR);
	}
	std::cerr << "Sent: " << bytes << " bytes" << std::endl;
	this->_context.responseBuffer.erase(0, bytes);

	if (0 == this->_context.responseBuffer.length() && IS_REQ_PROCESS_COMPLETE(this->_context.requestState)) {
		return (REQ_DONE);
	}
	std::cerr << "Response not fully sent" << std::endl;
	return (REQ_CONTINUE);
}

const ServerBlock *Client::findServerBlock(fd_t fd, const std::string &host) const {
	const std::vector<ServerBlock> &blocks = ((servermap_t *)(this->_context.ruleBlock))->at(fd);
	for (uint32_t i = 0; i < blocks.size(); ++i) {
		const std::vector<std::string> &names = blocks[i].names();
		for (uint32_t j = 0; j < names.size(); ++j) {
			if (0 == host.compare(names[j])) {
				return &blocks[i];
			}
		}
	}
	return &blocks.front();
}

/* ************************************************************************** */

error_t	Client::init(const fd_t serverSocket, const int32_t requestSocket, const servermap_t *serverBlocks)
{
	this->_context.ruleBlock = (void *)serverBlocks;
	this->_socket = requestSocket;
	this->_serverSocket = serverSocket;
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = this->_socket;
	if (-1 == epoll_ctl(Client::_epollFd, EPOLL_CTL_ADD, this->_socket, &event)) {
		close(this->_socket);
		return (-1);
	}
	std::cerr << "Client accepted! fd=" << this->_socket << std::endl;
	this->_context.requestState = REQ_STATE_NONE;
	return (0);
}

error_t	Client::handle(void)
{
	error_t	ret;

	if (!IS_REQ_READ_COMPLETE(this->_context.requestState)
		&& (ret = this->_readSocket()) != REQ_CONTINUE)
		return (ret);
	
	if (!IS_REQ_CLIENT_READ_COMPLETE(this->_context.requestState)
		&& (ret = this->_parseRequest()) != REQ_DONE)
		return (ret);

	// Handle request
	if (this->_request && !IS_REQ_PROCESS_COMPLETE(this->_context.requestState)) {
		ret = this->_process();
		if (ret != REQ_DONE)
			return (ret);
	}

	if (!IS_REQ_CAN_WRITE(this->_context.requestState))
	{
		if (this->_switchToWrite() == -1)
			return (REQ_ERROR);
	}

	// if (IS_REQ_CAN_WRITE(this->_context.requestState))	// possibly not needed
	// {
		if ((ret = this->_sendResponse()) != REQ_DONE)
			return (ret);
	// }

	std::cerr << "Natural exit: " << this->_requestStateStr() << std::endl;

	// if (IS_REQ_WRITE_COMPLETE(this->_requestState))
	// {
		if (-1 == epoll_ctl(Client::_epollFd, EPOLL_CTL_DEL, this->_socket, NULL))
		{
			close(this->_socket);
			return (REQ_ERROR);
		}
		close(this->_socket);
		return (REQ_DONE);
	// }
	// return (REQ_CONTINUE);
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

// void	Client::skipNextRead(void) { this->_skipNextRead = true; }

void	Client::setEpollFd(const int32_t fd) { Client::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */
