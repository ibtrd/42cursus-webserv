#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <arpa/inet.h>

#include "RequestGET.hpp"
#include "RequestPOST.hpp"
#include "RequestDELETE.hpp"
#include "Server.hpp"
#include "RequestPUT.hpp"
#include "webservHTML.hpp"
#include "fcntl.h"
#include "ft.hpp"

char	Client::_readBuffer[REQ_BUFFER_SIZE];

int32_t	Client::_epollFd = -1;

ARequest	*(*Client::_requestsBuilder[METHOD_INVAL_METHOD])(RequestContext_t &) = {
	createRequestGET,
	createRequestPOST,
	createRequestDELETE,
	createRequestPUT,
};

/* CONSTRUCTORS ************************************************************* */

Client::Client(const fd_t idSocket, const fd_t requestSocket, const Server &server, const struct sockaddr_in &addr) :
	_timestamp(time(NULL)),
	_idSocket(idSocket),
	_socket(requestSocket),
	_addr(addr),
	_request(NULL),
	_context(server),
	_bytesSent(0)
{
	// std::cerr << "Client created" << std::endl;
	this->_context.requestState = REQ_STATE_NONE;
	this->_request = NULL;
}

Client::Client(const Client &other) :
	_timestamp(other._timestamp),
	_idSocket(other._idSocket),
	_socket(other._socket),
	_addr(other._addr),
	_request(other._request),
	_context(other._context),
	// _errorPage(other._errorPage),
	_bytesSent(other._bytesSent)
{
	// std::cerr << "Client copy" << std::endl;
	*this = other;
}

Client::~Client(void)
{
	// std::cerr << "Client destroyed" << std::endl;
	// if (this->)
	std::cout << *this;
	if (this->_request)
		delete this->_request;
}

/* OPERATOR OVERLOADS ******************************************************* */

Client	&Client::operator=(const Client &other)
{
	// std::cerr << "Client assign" << std::endl;
	if (this == &other)
		return (*this);

	if (this->_request)
		delete this->_request;
	if (other._request)
		this->_request = other._request->clone();
	else
		this->_request = NULL;

	this->_context.serverBlock = other._context.serverBlock;
	this->_context.ruleBlock = other._context.ruleBlock;
	this->_context.requestState = other._context.requestState;
	this->_context.buffer = other._context.buffer;
	this->_context.method = other._context.method;
	this->_context.target = other._context.target;
	this->_context.protocolVersion = other._context.protocolVersion;
	this->_context.headers = other._context.headers;
	this->_context.response = other._context.response;
	this->_context.responseBuffer = other._context.responseBuffer;
	// this->_errorPage = other._errorPage;
	this->_bytesSent = other._bytesSent;
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
	str += ", processIn: ";
	if (IS_REQ_PROCESS_IN_COMPLETE(this->_context.requestState))
		str += "1";
	else
		str += "0";
	str += ", processOut: ";
	if (IS_REQ_PROCESS_OUT_COMPLETE(this->_context.requestState))
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

	// SET_REQ_CLIENT_READ_COMPLETE(this->_context.requestState);

	if (this->_context.response.statusCode() == STATUS_NONE) {
		ret = this->_resolveARequest();
		if (ret != REQ_CONTINUE)
			return (ret);
	}
	SET_REQ_READ_COMPLETE(this->_context.requestState); // probably not needed
	SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

error_t Client::_parseRequestLine(void)
{
	std::string	requestLine;

	// Check at least one line is present
	size_t	pos = this->_context.buffer.find("\r\n");
	if (pos == std::string::npos)
		return (REQ_CONTINUE);
	requestLine = this->_context.buffer.substr(0, pos);
	this->_context.buffer.erase(0, pos + 2);

	// Parse request line

	// Method
	pos = requestLine.find(' ');
	if (pos == std::string::npos)
	{
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	std::string	method = requestLine.substr(0, pos);
	if (method.empty())
	{
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	this->_context.method = Method(method);
	requestLine.erase(0, pos + 1);
	if (!this->_context.method.isValid())
	{
		this->_context.response.setStatusCode(STATUS_METHOD_NOT_ALLOWED);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	// Target
	pos = requestLine.find(' ');
	if (pos == std::string::npos)
	{
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	this->_context.target = requestLine.substr(0, pos);
	if (this->_context.target.empty() || this->_context.target[0] != '/')
	{
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	requestLine.erase(0, pos + 1);

	// Protocol version
	this->_context.protocolVersion = requestLine;
	if (this->_context.protocolVersion.empty())
	{
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (this->_context.protocolVersion != PROTOCOLE_VERSION)
	{
		this->_context.response.setStatusCode(STATUS_HTTP_VERSION_NOT_SUPPORTED);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

		// std::cerr << "method_t: |" << this->_context.method.string() << "|" << std::endl;
		// std::cerr << "Target: |" << this->_context.target << "|" << std::endl;
		// std::cerr << "Protocol version: |" << this->_context.protocolVersion << "|" << std::endl;

	SET_REQ_READ_REQUEST_LINE_COMPLETE(this->_context.requestState);

	return (REQ_DONE);
}

error_t Client::_parseHeaders(void)
{
	size_t	pos;
	std::string	line;
	std::string	key;
	std::string	value;

	// std::cerr << "Parsing headers..." << std::endl;
	while ((pos = this->_context.buffer.find("\r\n")) != std::string::npos)
	{
		line = this->_context.buffer.substr(0, pos);
		this->_context.buffer.erase(0, pos + 2);
		if (line.empty())
		{
			if (this->_context.headers.find(HEADER_HOST) == this->_context.headers.end())
			{
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_READ_COMPLETE(this->_context.requestState);
			}
			else
				SET_REQ_READ_HEADERS_COMPLETE(this->_context.requestState);
			return (REQ_DONE);
		}
		pos = line.find(": ");
		if (pos == std::string::npos)
		{
			this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
			SET_REQ_READ_COMPLETE(this->_context.requestState);
			return (REQ_DONE);
		}
		key = line.substr(0, pos);
		value = line.substr(pos + 2);
		this->_context.headers[key] = value;
		// std::cerr << "Header: |" << key << "| |" << value << "|" << std::endl;
	}
	return (REQ_CONTINUE);
}

error_t Client::_resolveARequest(void) {
	this->_context.serverBlock = &(this->_context.server
		.findServerBlock(this->_idSocket, this->_context.headers[HEADER_HOST]));
	this->_context.ruleBlock = (this->_context.serverBlock
		->findLocationBlock(this->_context.target));
	if (!this->_context.ruleBlock || this->_context.ruleBlock->getRoot().string().empty()) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
		return REQ_CONTINUE;
	}
	std::cerr << *this->_context.ruleBlock << std::endl;
	if (!this->_context.ruleBlock->isAllowed(this->_context.method)) {
		this->_context.response.setStatusCode(STATUS_METHOD_NOT_ALLOWED);
		return REQ_CONTINUE;
	}
	
	this->_request = Client::_requestsBuilder[this->_context.method.index()](this->_context);
	return REQ_DONE;
}

error_t	Client::_process(void)
{
	error_t	ret;

	if (!IS_REQ_READ_BODY_COMPLETE(this->_context.requestState))
	{
		ret = this->_request->parse();
		if (ret != REQ_DONE)
			return (ret);
	}

	if (!IS_REQ_PROCESS_IN_COMPLETE(this->_context.requestState))
	{
		ret = this->_request->processIn();
		if (ret != REQ_DONE)
			return (ret);
	}

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
	if (bytes > 0)
	{
		// std::cerr << this->_context.responseBuffer.substr(0, bytes) << std::endl;
		bytes = send(this->_socket, this->_context.responseBuffer.c_str(), bytes, MSG_NOSIGNAL);
		if (bytes == -1) {
			std::cerr << "Error: send: " << strerror(errno) << std::endl;
			return (REQ_ERROR);
		}
		this->_bytesSent += bytes;
		std::cerr << "Sent: " << bytes << " bytes" << std::endl;
		this->_context.responseBuffer.erase(0, bytes);
	}

	if (0 == this->_context.responseBuffer.length() && IS_REQ_PROCESS_COMPLETE(this->_context.requestState)) {
		return (REQ_DONE);
	}
	std::cerr << "Response not fully sent" << std::endl;
	return (REQ_CONTINUE);
}

void	Client::_loadErrorPage(void)
{
	// std::cerr << "Loading error page" << std::endl;

	const Path *errorPathPtr = NULL;
	Path	errorPath;

	if (!this->_context.serverBlock) {
		goto to_default_error_page;
	}
	errorPathPtr = this->_context.serverBlock->findErrorPage(this->_context.response.statusCode());
	if (!errorPathPtr) {
		goto to_default_error_page;
	}
	errorPath = *errorPathPtr;
	if (0 != errorPath.access(F_OK)) {
		goto to_default_error_page;
	}
	if (0 != errorPath.stat()) {
		goto to_default_error_page;
	}
	if (!errorPath.hasPermission(R_OK)) {
		goto to_default_error_page;
	}
	if (!errorPath.isFile()) {
		goto to_default_error_page;
	}
	this->_errorPage.open(errorPath.string().c_str(), std::ios::in | std::ios::binary);
	if (!this->_errorPage.is_open()) {
		goto to_default_error_page;
	}
	SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
	return ;

to_default_error_page:
	std::string errorBody;
	errorBody = HTMLERROR(ft::numToStr(this->_context.response.statusCode()), statusCodeToMsg(this->_context.response.statusCode()));
	this->_context.response.setBody(errorBody);
	SET_REQ_PROCESS_COMPLETE(this->_context.requestState);
}

void	Client::_readErrorPage(void)
{
	char	buffer[REQ_BUFFER_SIZE];

	this->_errorPage.read(buffer, REQ_BUFFER_SIZE);
	ssize_t	bytes = this->_errorPage.gcount();
	if (bytes == 0)
	{
		SET_REQ_PROCESS_OUT_COMPLETE(this->_context.requestState);
		return ;
	}

	this->_context.responseBuffer.append(buffer, bytes);
	return ;
}

error_t	Client::_handleSocketIn(void)
{
	error_t	ret;

	if (!IS_REQ_READ_COMPLETE(this->_context.requestState)
		&& (ret = this->_readSocket()) != REQ_CONTINUE)
		return (ret);
	
	if (!IS_REQ_CLIENT_READ_COMPLETE(this->_context.requestState)
		&& (ret = this->_parseRequest()) != REQ_DONE)
		return (ret);

	// Handle request
	if (this->_request && !IS_REQ_PROCESS_IN_COMPLETE(this->_context.requestState)) {
		ret = this->_process();
		if (ret != REQ_DONE)
			return (ret);
	}

	if (this->_context.response.statusCode() >= 400 && this->_context.response.statusCode() < 600) {
		this->_loadErrorPage();
	}

	this->_context.responseBuffer = this->_context.response.response();
	this->_context.response.clearBody();

	if (this->_switchToWrite() == -1)
		return (REQ_ERROR);
	return (REQ_CONTINUE);
}

error_t	Client::_handleSocketOut(void)
{
	error_t	ret;

	if (!IS_REQ_PROCESS_OUT_COMPLETE(this->_context.requestState) && this->_context.response.statusCode() >= 400 && this->_context.response.statusCode() < 600) {
		this->_readErrorPage();
	} else if (!IS_REQ_PROCESS_OUT_COMPLETE(this->_context.requestState) && this->_request) {
		ret = this->_request->processOut();
		// if (ret != REQ_DONE)
		// 	return (ret);
	}

	if ((ret = this->_sendResponse()) != REQ_DONE)
		return (ret);

	std::cerr << "Natural exit: " << this->_requestStateStr() << std::endl;

	if (-1 == epoll_ctl(Client::_epollFd, EPOLL_CTL_DEL, this->_socket, NULL))
	{
		close(this->_socket);
		return (REQ_ERROR);
	}
	close(this->_socket);
	return (REQ_DONE);
}

error_t	Client::_handleCGIIn(void)
{
	std::cerr << "CGI in" << std::endl;
	return (REQ_ERROR);
}

error_t	Client::_handleCGIOut(void)
{
	std::cerr << "CGI out" << std::endl;
	return (REQ_ERROR);
}

/* ************************************************************************** */

error_t	Client::init(void)
{
	this->_context.requestState = REQ_STATE_NONE;
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = this->_socket;
	if (-1 == epoll_ctl(Client::_epollFd, EPOLL_CTL_ADD, this->_socket, &event)) {
		close(this->_socket);
		return (-1);
	}
	std::cerr << "Client(" << this->_socket << ") accepted!" << std::endl;
	return (0);
}

error_t	Client::handleIn(fd_t fd)
{
	if (fd == this->_socket)
		return (this->_handleSocketIn());
	else
		return (this->_handleCGIIn());
}

error_t	Client::handleOut(fd_t fd)
{
	if (fd == this->_socket)
		return (this->_handleSocketOut());
	else
		return (this->_handleCGIOut());
}

error_t Client::timeoutCheck(const time_t now) {
	if (IS_REQ_READ_BODY_COMPLETE(this->_context.requestState)) {
			return (REQ_CONTINUE);
	}
	if (now - this->_timestamp >= REQUEST_TIMEOUT) {
		std::cerr << "Client(" << this->_socket << ") timeout detected!" << std::endl; //DEBUG
		this->_context.response.setStatusCode(STATUS_REQUEST_TIMEOUT);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		SET_REQ_PROCESS_IN_COMPLETE(this->_context.requestState);
		this->_loadErrorPage();

		this->_context.responseBuffer = this->_context.response.response();
		this->_context.response.clearBody();

		if (this->_switchToWrite() == -1)
			return (REQ_ERROR);
	}
	return (REQ_CONTINUE);
}

/* GETTERS ****************************************************************** */

fd_t	Client::socket(void) const { return this->_socket; }

void	Client::sockets(fd_t fds[2]) const { fds[0] = this->_socket; fds[1] = -1; }

time_t Client::timestamp(void) const {
	return this->_timestamp;
}

/* SETTERS ****************************************************************** */

void	Client::setEpollFd(const int32_t fd) { Client::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */

/* ************************************************************************** */

std::ostream &operator<<(std::ostream &os, const Client &client) {
	char buffer[128];
	std::strftime(buffer, sizeof(buffer), "%c", std::localtime(&client._timestamp));
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client._addr.sin_addr, clientIP, INET_ADDRSTRLEN);
	os << clientIP << " [" << buffer << "] ";
	if (client._context.method.isValid()) {
		os << '"' << client._context.method.string() << " "
			<< client._context.target << " "
			<< client._context.protocolVersion << "\" ";
	}
	os << client._context.response.statusCode() << " "
		<< client._bytesSent;
	headers_t::const_iterator agent = client._context.headers.find(HEADER_USER_AGENT);
	if (agent != client._context.headers.end()) {
		os << " \"" << agent->second << '"';
	}
	os << std::endl;
	return os;
}
