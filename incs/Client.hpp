#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ARequest.hpp"
# include "ServerBlock.hpp"

# include <string>
# include <map>

# define PROTOCOLE_VERSION "HTTP/1.1"

class Client {
private:
	static	char		_readBuffer[REQ_BUFFER_SIZE];
	static	int32_t		_epollFd;
	static	ARequest	*(*_requestsBuilder[INVAL_METHOD])(RequestContext_t &);
	
	const fd_t			_idSocket;
	const fd_t			_socket;

	ARequest			*_request;
	RequestContext_t	_context;

	const std::string	_requestStateStr(void) const;

	error_t				_readSocket(void);
	error_t				_parseRequest(void);
	error_t				_parseRequestLine(void);
	error_t				_parseHeaders(void);
	error_t				_process(void);
	error_t				_switchToWrite(void);
	error_t				_sendResponse(void);
	error_t				_handleSocketIn(void);
	error_t				_handleSocketOut(void);
	error_t				_handleFileIn(void);
	error_t				_handleFileOut(void);
	error_t				_resolveARequest(void);
	const LocationBlock	*_findRuleBlock(void);

public:
	Client(const fd_t idSocket, const fd_t requestSocket, Server const &server);
	Client(const Client &other);

	~Client(void);

	Client	&operator=(const Client &other);

	// error_t		init(const fd_t idSocket, const fd_t requestSocket, Server const *server);
	error_t		init(void);
	error_t		handleIn(fd_t fd);
	error_t		handleOut(fd_t fd);

	// GETTERS
	void	sockets(fd_t fds[2]) const;

	// SETTERS
	static void	setEpollFd(const int32_t fd);
};

#endif /* ******************************************************************* */
