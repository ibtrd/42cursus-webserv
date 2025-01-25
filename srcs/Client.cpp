#include <arpa/inet.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "RequestDELETE.hpp"
#include "RequestGET.hpp"
#include "RequestHEAD.hpp"
#include "RequestPOST.hpp"
#include "RequestPUT.hpp"
#include "Server.hpp"
#include "fcntl.h"
#include "ft.hpp"
#include "webservHTML.hpp"

uint8_t Client::_readBuffer[REQ_BUFFER_SIZE];

int32_t Client::_epollFd = -1;

ARequest *(*Client::_requestsBuilder[METHOD_INVAL_METHOD])(RequestContext_t &) = {
    createRequestGET, createRequestPOST, createRequestDELETE, createRequestPUT, createRequestHEAD,
};

/* CONSTRUCTORS ************************************************************* */

Client::Client(const fd_t idSocket, const fd_t requestSocket, Server &server,
               const struct sockaddr_in &addr)
    : _idSocket(idSocket),
    //   _socket(requestSocket),
      _request(NULL),
      _context(server, addr),
      _bytesSent(0) {
	// std::cerr << "Client created" << std::endl;
	if (server.getTimeout(CLIENT_HEADER_TIMEOUT)) {
		this->_timestamp[CLIENT_HEADER_TIMEOUT] = time(NULL);
	}
	this->_timestamp[CLIENT_BODY_TIMEOUT] = std::numeric_limits<time_t>::max();
	this->_timestamp[SEND_TIMEOUT]        = std::numeric_limits<time_t>::max();
	this->_clientEvent.events             = EPOLLIN;
	this->_clientEvent.data.fd            = requestSocket;
	this->_context._cgiSockets[PARENT_SOCKET] = -1;
	this->_context._cgiSockets[CHILD_SOCKET]  = -1;
	this->_context._pid                       = -1;
	this->_context.requestState           = REQ_STATE_NONE;
	this->_request                        = NULL;
}

Client::Client(const Client &other)
    : _idSocket(other._idSocket),
      _request(other._request),
      _context(other._context),
      _bytesSent(other._bytesSent) {
	std::cerr << "[WARNING] Client copy" << std::endl;	// Not updated

	for (int i = 0; i < TIMEOUT_COUNT; ++i) {
		this->_timestamp[i] = other._timestamp[i];
	}

	this->_clientEvent.events = other._clientEvent.events;
	this->_clientEvent.data   = other._clientEvent.data;

	// context
	this-> _context = other._context;
}

Client::~Client(void) {
	// std::cerr << "Client destroyed" << std::endl;
	if (this->_request) {
		delete this->_request;
	}
	// if (this->_context._cgiSockets[PARENT_SOCKET] != -1) {
	// 	close(this->_context._cgiSockets[PARENT_SOCKET]);
	// }
	// if (this->_context._cgiSockets[CHILD_SOCKET] != -1) {
	// 	close(this->_context._cgiSockets[CHILD_SOCKET]);
	// }
}

/* OPERATOR OVERLOADS ******************************************************* */

Client &Client::operator=(const Client &other) {
	std::cerr << "[WARNING] Client assign" << std::endl;	// Not updated
	if (this == &other) {
		return (*this);
	}

	if (this->_request) {
		delete this->_request;
	}
	if (other._request) {
		this->_request = other._request->clone();
	} else {
		this->_request = NULL;
	}

	for (int i = 0; i < TIMEOUT_COUNT; ++i) {
		this->_timestamp[i] = other._timestamp[i];
	}

	this->_context = other._context;

	this->_bytesSent = other._bytesSent;
	return (*this);
}

/* ************************************************************************** */

const std::string Client::_requestStateStr(void) const {
	std::string str("{");

	str += "requestLine: ";
	if (IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_context.requestState)) {
		str += "1";
	} else {
		str += "0";
	}
	str += ", headers: ";
	if (IS_REQ_READ_HEADERS_COMPLETE(this->_context.requestState)) {
		str += "1";
	} else {
		str += "0";
	}
	str += ", clientRead: ";
	if (IS_REQ_CLIENT_READ_COMPLETE(this->_context.requestState)) {
		str += "1";
	} else {
		str += "0";
	}
	str += ", read: ";
	if (IS_REQ_READ_COMPLETE(this->_context.requestState)) {
		str += "1";
	} else {
		str += "0";
	}
	str += ", workIn: ";
	if (IS_REQ_WORK_IN_COMPLETE(this->_context.requestState)) {
		str += "1";
	} else {
		str += "0";
	}
	str += ", workOut: ";
	if (IS_REQ_WORK_OUT_COMPLETE(this->_context.requestState)) {
		str += "1";
	} else {
		str += "0";
	}
	str += ", writing: ";
	if (IS_REQ_CAN_WRITE(this->_context.requestState)) {
		str += "1";
	} else {
		str += "0";
	}
	str += ", writeComplete: ";
	if (IS_REQ_WRITE_COMPLETE(this->_context.requestState)) {
		str += "1";
	} else {
		str += "0";
	}
	str += "}";
	return (str);
}

error_t Client::_readSocket(void) {
	ssize_t bytes;
	bytes = recv(this->_clientEvent.data.fd, Client::_readBuffer, REQ_BUFFER_SIZE, 0);
	if (bytes == 0) {
		std::cerr << "Client disconnected" << std::endl;
		return (REQ_DONE);
	}
	if (bytes == -1) {
		std::cerr << "Client error" << std::endl;
		return (REQ_ERROR);
	}
	this->_context.buffer.append(Client::_readBuffer, bytes);
	return (REQ_CONTINUE);
}

error_t Client::_parseRequest(void) {
	error_t ret;

	// Parse request line
	if (!IS_REQ_READ_REQUEST_LINE_COMPLETE(this->_context.requestState)) {
		ret = this->_parseRequestLine();
		if (ret != REQ_DONE) {
			return (ret);
		}
	}

	// Parse headers
	if (!IS_REQ_READ_HEADERS_COMPLETE(this->_context.requestState)) {
		ret = this->_parseHeaders();
		if (ret != REQ_DONE) {
			return (ret);
		}
	}

	if (this->_context.response.statusCode() == STATUS_NONE) {
		ret = this->_resolveARequest();
		if (ret != REQ_CONTINUE) {
			return (ret);
		}
	}

	SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);
	return (REQ_DONE);
}

error_t Client::_parseRequestLine(void) {
	std::string requestLine;

	// Check at least one line is present
	size_t pos = this->_context.buffer.find("\r\n");
	if (pos == std::string::npos) {
		return (REQ_CONTINUE);
	}
	requestLine = this->_context.buffer.substr(0, pos);
	this->_context.buffer.erase(0, pos + 2);

	// Parse request line

	// Method
	pos = requestLine.find(' ');
	if (pos == std::string::npos) {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	std::string method = requestLine.substr(0, pos);
	if (method.empty()) {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	this->_context.method = Method(method);
	requestLine.erase(0, pos + 1);
	if (!this->_context.method.isValid()) {
		this->_context.response.setStatusCode(STATUS_METHOD_NOT_ALLOWED);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	// Target
	pos = requestLine.find(' ');
	if (pos == std::string::npos) {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	this->_context.target = requestLine.substr(0, pos);
	if (this->_context.target.empty() || this->_context.target[0] != '/') {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	requestLine.erase(0, pos + 1);

	//  Query string
		pos = this->_context.target.find('?');
		if (pos != std::string::npos) {
			this->_context.queries = Queries(this->_context.target.substr(pos + 1));
			this->_context.target.erase(pos);
		}

	// Protocol version
	this->_context.protocolVersion = requestLine;
	if (this->_context.protocolVersion.empty()) {
		this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}
	if (this->_context.protocolVersion != PROTOCOLE_VERSION) {
		this->_context.response.setStatusCode(STATUS_HTTP_VERSION_NOT_SUPPORTED);
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		return (REQ_DONE);
	}

	std::cerr << "Method: |" << this->_context.method.string() << "|" << std::endl;
	std::cerr << "Target: |" << this->_context.target << "|" << std::endl;
	std::cerr << "Queries: |" << this->_context.queries.queryLine() << "|" << std::endl;
	std::cerr << "Protocol version: |" << this->_context.protocolVersion << "|" << std::endl;

	SET_REQ_READ_REQUEST_LINE_COMPLETE(this->_context.requestState);

	return (REQ_DONE);
}

error_t Client::_parseHeaders(void) {
	size_t      pos;
	std::string line;
	std::string key;
	std::string value;

	// std::cerr << "Parsing headers..." << std::endl;
	while ((pos = this->_context.buffer.find("\r\n")) != std::string::npos) {
		line = this->_context.buffer.substr(0, pos);
		this->_context.buffer.erase(0, pos + 2);
		if (line.empty()) {
			if (this->_context.headers.find(HEADER_HOST) == this->_context.headers.end()) {
				this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
				SET_REQ_READ_COMPLETE(this->_context.requestState);
			} else {
				SET_REQ_READ_HEADERS_COMPLETE(this->_context.requestState);
			}
			return (REQ_DONE);
		}
		pos = line.find(": ");
		if (pos == std::string::npos) {
			this->_context.response.setStatusCode(STATUS_BAD_REQUEST);
			SET_REQ_READ_COMPLETE(this->_context.requestState);
			return (REQ_DONE);
		}
		key                         = line.substr(0, pos);
		value                       = line.substr(pos + 2);
		this->_context.headers[key] = value;
		// std::cerr << "Header: |" << key << "| |" << value << "|" << std::endl;
	}
	return (REQ_CONTINUE);
}

error_t Client::_resolveARequest(void) {
	this->_context.serverBlock = &(this->_context.server.findServerBlock(
	    this->_idSocket, this->_context.headers[HEADER_HOST]));
	this->_context.ruleBlock =
	    (this->_context.serverBlock->findLocationBlock(this->_context.target));
	if (!this->_context.ruleBlock || this->_context.ruleBlock->getRoot().empty()) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
		return REQ_CONTINUE;
	}
	std::cerr << *this->_context.ruleBlock << std::endl;
	if (!this->_context.ruleBlock->isAllowed(this->_context.method)) {
		this->_context.response.setStatusCode(STATUS_METHOD_NOT_ALLOWED);
		return REQ_CONTINUE;
	}

	const redirect_t &redirect = this->_context.ruleBlock->getRedirect();
	if (redirect.first != STATUS_NONE) {
		this->_context.response.setStatusCode(redirect.first);
		this->_context.response.setHeader(HEADER_LOCATION, redirect.second);
		SET_REQ_WORK_COMPLETE(
		    this->_context.requestState);  // No work needed for redirect (response is ready)
		return REQ_DONE;
	}

	this->_request = Client::_requestsBuilder[this->_context.method.index()](this->_context);
	if (!this->_request) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return REQ_CONTINUE;
	}
	this->_request->processing();
	if (-1 != this->_context._cgiSockets[PARENT_SOCKET]) {
		// add to epoll
		struct epoll_event event;
		event.events  = EPOLLIN;
		event.data.fd = this->_context._cgiSockets[PARENT_SOCKET];
		if (-1 == epoll_ctl(Client::_epollFd, EPOLL_CTL_ADD, this->_context._cgiSockets[PARENT_SOCKET], &event)) {
			std::cerr << "Error: epoll_ctl: " << strerror(errno) << std::endl;
			return REQ_ERROR;
		}
		// add to server clientbindmap
		if (this->_context.server.addCGIToClientMap(this->_context._cgiSockets[PARENT_SOCKET], *this)) {
			std::cerr << "Error: addCGIToClientMap" << std::endl;
			return REQ_ERROR;
		}
	}
	if (this->_context.response.statusCode() != STATUS_NONE) {
		return REQ_CONTINUE;
	}
	return REQ_DONE;
}

error_t Client::_switchToWrite(void) {
	struct epoll_event event;
	event.events  = EPOLLOUT;
	event.data.fd = this->_clientEvent.data.fd;
	if (-1 == epoll_ctl(Client::_epollFd, EPOLL_CTL_MOD, this->_clientEvent.data.fd, &event)) {
		close(this->_clientEvent.data.fd);
		return (-1);
	}
	SET_REQ_CAN_WRITE(this->_context.requestState);
	return (0);
}

error_t Client::_sendResponse(void) {
	// std::cerr << "Sending response..." << std::endl;
	std::cerr << ".";
	ssize_t bytes;

	bytes                          = REQ_BUFFER_SIZE > this->_context.responseBuffer.size()
	                                     ? this->_context.responseBuffer.size()
	                                     : REQ_BUFFER_SIZE;
	if (bytes > 0) {
		// std::cerr << this->_context.responseBuffer.substr(0, bytes) << std::endl;
		bytes = send(this->_clientEvent.data.fd, this->_context.responseBuffer.c_str(), bytes, MSG_NOSIGNAL);
		if (bytes == -1) {
			std::cerr << "Error: send: " << strerror(errno) << std::endl;
			return (REQ_ERROR);
		}
		if (bytes > 0) {
			this->_timestamp[SEND_TIMEOUT] = time(NULL);
		}
		this->_bytesSent += bytes;
		// std::cerr << "Sent: " << bytes << " bytes" << std::endl;
		this->_context.responseBuffer.erase(0, bytes);
	}

	if (0 == this->_context.responseBuffer.size() &&
	    IS_REQ_WORK_COMPLETE(this->_context.requestState)) {
		return (REQ_DONE);
	}
	// std::cerr << "Response not fully sent" << std::endl;
	return (REQ_CONTINUE);
}

error_t Client::_openErrorPage(void) {
	// std::cerr << "Loading error page" << std::endl;

	if (!IS_REQ_WORK_IN_COMPLETE(this->_context.requestState)) {
		std::cerr << "[Info] page requested before workIn was marked completed" << std::endl;	// DEBUG
	} else {
		std::cerr << "[Info] page requested after workIn was marked completed" << std::endl;	// DEBUG
	}

	if (!this->_context.serverBlock) {
		return REQ_ERROR;
	}

	const Path *errorPathPtr =
	    this->_context.serverBlock->findErrorPage(this->_context.response.statusCode());
	if (!errorPathPtr) {
		return REQ_ERROR;
	}

	Path errorPath = *errorPathPtr;
	if (0 == errorPath.access(F_OK) && 0 == errorPath.stat() && 0 == errorPath.access(R_OK) &&
	    errorPath.isFile()) {
		this->_errorPage.open(errorPath.string().c_str(), std::ios::in | std::ios::binary);
		if (this->_errorPage.is_open()) {
			this->_context.response.setHeader(HEADER_CONTENT_LENGTH,
			                                  ft::numToStr(errorPath.size()));
			return REQ_DONE;
			UNSET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		}
	}
	return REQ_ERROR;
}

void Client::_loadErrorPage(void) {
	if (this->_openErrorPage() == REQ_ERROR) {
		std::string errorBody;
		errorBody = HTMLERROR(ft::numToStr(this->_context.response.statusCode()),
								statusCodeToMsg(this->_context.response.statusCode()));
		this->_context.response.setBody(errorBody);
		this->_context.response.setHeader(HEADER_CONTENT_LENGTH,
											ft::numToStr(errorBody.length()));
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
	}
}

void Client::_readErrorPage(void) {
	uint8_t buffer[REQ_BUFFER_SIZE];

	this->_errorPage.read((char *)buffer, REQ_BUFFER_SIZE);
	ssize_t bytes = this->_errorPage.gcount();
	if (bytes == 0) {
		SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		return;
	}

	this->_context.responseBuffer.append(buffer, bytes);
	return;
}

error_t Client::_handleSocketIn(void) {
	error_t ret;

	if (!IS_REQ_READ_COMPLETE(this->_context.requestState) &&
	    (ret = this->_readSocket()) != REQ_CONTINUE) {
		return (ret);
	}

	if (!IS_REQ_CLIENT_READ_COMPLETE(this->_context.requestState) &&
	    (ret = this->_parseRequest()) != REQ_DONE) {
		return (ret);
	}

	// Handle request
	if (this->_request && !IS_REQ_WORK_IN_COMPLETE(this->_context.requestState)) {
		this->_timestamp[CLIENT_BODY_TIMEOUT] = time(NULL);
		ret                                   = this->_request->workIn();
		if (ret != REQ_DONE) {
			return (ret);
		}
	}

	if (this->_context.response.statusCode() >= 400 && this->_context.response.statusCode() < 600) {
		this->_context.response.setHeader(HEADER_CONTENT_TYPE, "text/html");
		this->_loadErrorPage();
	}
	this->_context.responseBuffer = this->_context.response.response();
	this->_context.response.clearBody();

	if (this->_switchToWrite() == -1) {
		return (REQ_ERROR);
	}
	return (REQ_CONTINUE);
}

error_t Client::_handleSocketOut(void) {
	error_t ret;

	if (!IS_REQ_WORK_OUT_COMPLETE(this->_context.requestState) &&
	    this->_context.response.statusCode() >= 400 && this->_context.response.statusCode() < 600) {
		this->_readErrorPage();
	} else if (!IS_REQ_WORK_OUT_COMPLETE(this->_context.requestState) && this->_request) {
		ret = this->_request->workOut();
		// if (ret != REQ_DONE)
		// 	return (ret);
	}

	if ((ret = this->_sendResponse()) != REQ_DONE) {
		return (ret);
	}
	std::cerr << "Natural exit: " << this->_requestStateStr() << std::endl;
	return (REQ_DONE);
}

error_t Client::_handleCGIIn(void) {
	std::cerr << "CGI in" << std::endl;
	return (this->_request->CGIIn());
}

error_t Client::_handleCGIOut(void) {
	std::cerr << "CGI out" << std::endl;
	return (this->_request->CGIOut());
}

/* ************************************************************************** */

error_t Client::init(void) {
	this->_context.requestState = REQ_STATE_NONE;

	this->_clientEvent.events  = EPOLLIN;
	if (-1 == epoll_ctl(Client::_epollFd, EPOLL_CTL_ADD, this->_clientEvent.data.fd, &this->_clientEvent)) {
		close(this->_clientEvent.data.fd);
		return (-1);
	}
	std::cerr << "Client(" << this->_clientEvent.data.fd << ") accepted!" << std::endl;
	return (0);
}

error_t Client::handleIn(fd_t fd) {
	if (fd == this->_clientEvent.data.fd) {
		return (this->_handleSocketIn());
	} else {
		return (this->_handleCGIIn());
	}
}

error_t Client::handleOut(fd_t fd) {
	if (fd == this->_clientEvent.data.fd) {
		return (this->_handleSocketOut());
	} else {
		return (this->_handleCGIOut());
	}
}

error_t Client::timeoutCheck(const time_t now) {
	if (!IS_REQ_READ_HEADERS_COMPLETE(this->_context.requestState) &&
	    this->_context.server.getTimeout(CLIENT_HEADER_TIMEOUT) &&
	    now - this->_timestamp[CLIENT_HEADER_TIMEOUT] >=
	        this->_context.server.getTimeout(CLIENT_HEADER_TIMEOUT)) {
		std::cerr << "Client(" << this->_clientEvent.data.fd << ") header timeout detected!"
		          << std::endl;  // DEBUG

		this->_context.response.setStatusCode(STATUS_REQUEST_TIMEOUT);
		this->_context.response.disableIsCgi();
		SET_REQ_READ_COMPLETE(this->_context.requestState);
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
		UNSET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		this->_loadErrorPage();

		this->_context.responseBuffer = this->_context.response.response();
		this->_context.response.clearBody();

		if (this->_switchToWrite() == -1) {
			return (REQ_ERROR);
		}
		return (REQ_CONTINUE);
	}

	if (!IS_REQ_WORK_IN_COMPLETE(this->_context.requestState) &&
	    this->_context.server.getTimeout(CLIENT_BODY_TIMEOUT) &&
	    now - this->_timestamp[CLIENT_BODY_TIMEOUT] >=
	        this->_context.server.getTimeout(CLIENT_BODY_TIMEOUT)) {
		std::cerr << "Client(" << this->_clientEvent.data.fd << ") body timeout detected!"
		          << std::endl;  // DEBUG

		this->_context.response.setStatusCode(STATUS_REQUEST_TIMEOUT);
		this->_context.response.disableIsCgi();
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
		UNSET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		this->_loadErrorPage();

		this->_context.responseBuffer = this->_context.response.response();
		this->_context.response.clearBody();

		if (this->_switchToWrite() == -1) {
			return (REQ_ERROR);
		}
		return (REQ_CONTINUE);
	}

	if (IS_REQ_CAN_WRITE(this->_context.requestState) &&
	    this->_context.server.getTimeout(SEND_TIMEOUT) &&
	    now - this->_timestamp[SEND_TIMEOUT] >= this->_context.server.getTimeout(SEND_TIMEOUT)) {
		std::cerr << "Client(" << this->_clientEvent.data.fd << ") send timeout detected!"
		          << std::endl;  // DEBUG
		return (REQ_DONE);
	}

	return (REQ_CONTINUE);
}

/* GETTERS ****************************************************************** */

struct epoll_event Client::clientEvent(void) const { return this->_clientEvent; }

fd_t Client::socket(void) const { return this->_clientEvent.data.fd; }

void Client::sockets(fd_t fds[2]) const {
	fds[0] = this->_clientEvent.data.fd;
	fds[1] = this->_context._cgiSockets[PARENT_SOCKET];
}

pid_t Client::cgiPid(void) const { return this->_context._pid; }

/* SETTERS ****************************************************************** */

void Client::setEpollFd(const int32_t fd) { Client::_epollFd = fd; }

/* EXCEPTIONS *************************************************************** */

/* ************************************************************************** */

std::ostream &operator<<(std::ostream &os, const Client &client) {
	char buffer[128];
	std::strftime(buffer, sizeof(buffer), "%c",
	              std::localtime(&client._timestamp[CLIENT_HEADER_TIMEOUT]));
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client._context.addr.sin_addr, clientIP, INET_ADDRSTRLEN);
	os << clientIP << " [" << buffer << "] ";
	if (client._context.method.isValid()) {
		os << '"' << client._context.method.string() << " " << client._context.target << " "
		   << client._context.protocolVersion << "\" ";
	}
	os << client._context.response.statusCode() << " " << client._bytesSent;
	headers_t::const_iterator agent = client._context.headers.find(HEADER_USER_AGENT);
	if (agent != client._context.headers.end()) {
		os << " \"" << agent->second << '"';
	}
	os << std::endl;
	return os;
}

bool Client::operator==(const Client &other) const { return (this->_idSocket == other._idSocket); }
