#include <arpa/inet.h>
#include <unistd.h>

#include "RequestDELETE.hpp"
#include "RequestGET.hpp"
#include "RequestHEAD.hpp"
#include "RequestPOST.hpp"
#include "RequestPUT.hpp"
#include "Server.hpp"
#include "ft.hpp"

uint8_t Client::_readBuffer[REQ_BUFFER_SIZE];

int32_t Client::epollFd = -1;

ARequest *(*Client::_requestsBuilder[METHOD_INVAL_METHOD])(RequestContext_t &) = {
    createRequestGET, createRequestPOST, createRequestDELETE, createRequestPUT, createRequestHEAD,
};

/* CONSTRUCTORS ************************************************************* */

Client::Client(const fd_t idSocket, const fd_t requestSocket, Server &server,
               const struct sockaddr_in &addr)
    : _connectSocket(idSocket), _request(NULL), _context(server, addr), _bytesSent(0) {
	if (server.getTimeout(CLIENT_HEADER_TIMEOUT)) {
		this->_timestamp[CLIENT_HEADER_TIMEOUT] = time(NULL);
	}
	this->_timestamp[CLIENT_BODY_TIMEOUT] = std::numeric_limits<time_t>::max();
	this->_timestamp[SEND_TIMEOUT]        = std::numeric_limits<time_t>::max();
	this->_clientEvent.events             = EPOLLIN;
	this->_clientEvent.data.fd            = requestSocket;
	this->_request                        = NULL;
}

Client::Client(const Client &other)
    : _connectSocket(other._connectSocket),
      _request(other._request),
      _context(other._context),
      _bytesSent(other._bytesSent) {
	std::cerr << "[WARNING] Client(" << other._clientEvent.data.fd << ") copy"
	          << std::endl;  // Not updated

	for (int i = 0; i < TIMEOUT_COUNT; ++i) {
		this->_timestamp[i] = other._timestamp[i];
	}

	this->_clientEvent.events = other._clientEvent.events;
	this->_clientEvent.data   = other._clientEvent.data;
	this->_context = other._context;
}

Client::~Client(void) {
	if (this->_request) {
		delete this->_request;
	}
	std::cerr << "Destroy Status: " << this->_requestStateStr() << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

bool Client::operator==(const Client &other) const {
	return (this->_clientEvent.data.fd == other._clientEvent.data.fd);
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
	str += ", cgiIn: ";
	if (IS_REQ_CGI_IN_COMPLETE(this->_context.requestState)) {
		str += "1";
	} else {
		str += "0";
	}
	str += ", cgiOut: ";
	if (IS_REQ_CGI_OUT_COMPLETE(this->_context.requestState)) {
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
	str += ", cgiHeaders: ";
	if (IS_REQ_CGI_HEADERS_COMPLETE(this->_context.requestState)) {
		str += "1";
	} else {
		str += "0";
	}
	str += "}";
	return (str);
}

error_t Client::_readSocket(void) {
	ssize_t bytes;
	bytes = recv(this->_clientEvent.data.fd, Client::_readBuffer, REQ_BUFFER_SIZE, MSG_NOSIGNAL);
	if (bytes == 0) {
		std::cerr << "Client disconnected" << std::endl;
		return (REQ_DONE);
	}
	if (bytes == -1) {
		std::cerr << "Client error" << std::endl;
		return (REQ_ERROR);
	}
	this->_context.buffer.append(Client::_readBuffer, bytes);
	if (IS_REQ_CLIENT_READ_COMPLETE(this->_context.requestState)) {
		std::cerr << "TIMEOUT UPDATE" << std::endl;
		this->_timestamp[CLIENT_BODY_TIMEOUT] = time(NULL);
	}
	return (REQ_CONTINUE);
}

error_t Client::_resolveARequest(void) {
	this->_context.serverBlock = &(this->_context.server.findServerBlock(
	    this->_connectSocket, this->_context.headers[HEADER_HOST]));
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
	if (-1 != this->_context.cgiSockets[PARENT_SOCKET]) {
		// add to epoll
		struct epoll_event event;
		event.events = this->_context.option;
		std::cerr << "Adding CGI socket to epoll: " << this->_context.cgiSockets[PARENT_SOCKET]
		          << " with option: " << this->_context.option << std::endl;
		event.data.fd = this->_context.cgiSockets[PARENT_SOCKET];
		if (-1 == epoll_ctl(Client::epollFd, EPOLL_CTL_ADD,
		                    this->_context.cgiSockets[PARENT_SOCKET], &event)) {
			throw std::runtime_error("epoll_ctl(): " + std::string(strerror(errno)));
			return REQ_ERROR;
		}
		// add to server clientbindmap
		if (this->_context.server.addCGIToClientMap(this->_context.cgiSockets[PARENT_SOCKET],
		                                            *this)) {
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
	event.events  = EPOLLOUT | EPOLLIN;
	event.data.fd = this->_clientEvent.data.fd;
	if (-1 == epoll_ctl(Client::epollFd, EPOLL_CTL_MOD, this->_clientEvent.data.fd, &event)) {
		throw std::runtime_error("epoll_ctl(): " + std::string(strerror(errno)));
		return (-1);
	}
	SET_REQ_CAN_WRITE(this->_context.requestState);
	return (0);
}

error_t Client::_sendResponse(void) {
	ssize_t bytes;

	bytes = REQ_BUFFER_SIZE > this->_context.responseBuffer.size()
	            ? this->_context.responseBuffer.size()
	            : REQ_BUFFER_SIZE;
	if (bytes > 0) {
		bytes = send(this->_clientEvent.data.fd, this->_context.responseBuffer.c_str(), bytes,
		             MSG_NOSIGNAL);
		if (bytes == -1) {
			std::cerr << "Error: send: " << strerror(errno) << std::endl;
			return (REQ_ERROR);
		}
		if (bytes > 0) {
			this->_timestamp[SEND_TIMEOUT] = time(NULL);
		}
		this->_bytesSent += bytes;
		this->_context.responseBuffer.erase(0, bytes);
	}

	if (0 == this->_context.responseBuffer.size() &&
	    IS_REQ_WORK_COMPLETE(this->_context.requestState)) {
		return (REQ_DONE);
	}
	return (REQ_CONTINUE);
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

	if (IS_REQ_CGI_IN_COMPLETE(this->_context.requestState)) {
		this->_context.responseBuffer = this->_context.response.response();
		this->_context.response.clearBody();
	}

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
	}
	if ((ret = this->_sendResponse()) != REQ_DONE) {
		return (ret);
	}
	return (REQ_DONE);
}

error_t Client::_handleCGIIn(void) {
	return (this->_request->CGIIn());
}

error_t Client::_handleCGIOut(void) {
	return (this->_request->CGIOut());
}

/* ************************************************************************** */

error_t Client::init(void) {
	this->_context.requestState = REQ_STATE_NONE;

	this->_clientEvent.events = EPOLLIN;
	if (-1 == epoll_ctl(Client::epollFd, EPOLL_CTL_ADD, this->_clientEvent.data.fd,
	                    &this->_clientEvent)) {
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

	if ((!IS_REQ_WORK_IN_COMPLETE(this->_context.requestState) ||
	     !IS_REQ_CGI_OUT_COMPLETE(this->_context.requestState)) &&
	    this->_context.server.getTimeout(CLIENT_BODY_TIMEOUT) &&
	    now - this->_timestamp[CLIENT_BODY_TIMEOUT] >=
	        this->_context.server.getTimeout(CLIENT_BODY_TIMEOUT)) {
		std::cerr << "Client(" << this->_clientEvent.data.fd << ") body timeout detected!"
		          << std::endl;  // DEBUG

		this->_context.response.setStatusCode(STATUS_REQUEST_TIMEOUT);
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
	fds[1] = this->_context.cgiSockets[PARENT_SOCKET];
}

pid_t Client::cgiPid(void) const { return this->_context.pid; }

/* SETTERS ****************************************************************** */

void Client::setEpollFd(const int32_t fd) { Client::epollFd = fd; }

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
