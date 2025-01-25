#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/epoll.h>

#include <ctime>
#include <fstream>
#include <map>
#include <string>

#include "ARequest.hpp"
#include "ServerBlock.hpp"

#define PROTOCOLE_VERSION "HTTP/1.1"

class Client {
private:
	static uint8_t _readBuffer[REQ_BUFFER_SIZE];
	static int32_t _epollFd;
	static ARequest *(*_requestsBuilder[METHOD_INVAL_METHOD])(RequestContext_t &);

	time_t                   _timestamp[TIMEOUT_COUNT];
	const fd_t               _idSocket;
	struct epoll_event       _clientEvent;
	// const fd_t               _socket;

	ARequest        *_request;
	RequestContext_t _context;
	std::ifstream    _errorPage;
	size_t           _bytesSent;

	const std::string _requestStateStr(void) const;

	error_t _readSocket(void);

	error_t _parseRequest(void);

	error_t _parseRequestLine(void);
	error_t _parseHeaders(void);
	error_t _resolveARequest(void);

	error_t _switchToWrite(void);
	error_t _sendResponse(void);

	error_t _handleSocketIn(void);
	error_t _handleSocketOut(void);
	error_t _handleCGIIn(void);
	error_t _handleCGIOut(void);

	error_t _openErrorPage(void);
	void    _loadErrorPage(void);
	void    _readErrorPage(void);

	friend std::ostream &operator<<(std::ostream &os, const Client &client);

public:
	Client(const fd_t idSocket, const fd_t requestSocket, Server const &server,
	       const struct sockaddr_in &addr);
	Client(const Client &other);

	~Client(void);

	Client &operator=(const Client &other);

	// error_t		init(const fd_t idSocket, const fd_t requestSocket, Server const *server);
	error_t init(void);
	error_t handleIn(fd_t fd);
	error_t handleOut(fd_t fd);

	error_t timeoutCheck(const time_t now);

	// GETTERS
	struct epoll_event clientEvent(void) const;
	fd_t               socket(void) const;
	void               sockets(fd_t fds[2]) const;

	// SETTERS
	static void setEpollFd(const int32_t fd);
};

#endif /* ******************************************************************* */
