#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ARequest.hpp"
# include "ServerBlock.hpp"

# include <string>
# include <map>
# include <ctime>
# include <fstream>

# define PROTOCOLE_VERSION "HTTP/1.1"

class Client {
private:
	static	char		_readBuffer[REQ_BUFFER_SIZE];
	static	int32_t		_epollFd;
	static	ARequest	*(*_requestsBuilder[INVAL_METHOD])(RequestContext_t &);
	
	const time_t				_timestamp;
	const fd_t					_idSocket;
	const fd_t					_socket;
	const struct sockaddr_in	_addr;

	ARequest			*_request;
	RequestContext_t	_context;
	std::ifstream		_errorPage;
	size_t				_bytesSent;


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
	error_t				_handleCGIIn(void);
	error_t				_handleCGIOut(void);
	error_t				_resolveARequest(void);
	void				_loadErrorPage(void);
	void				_readErrorPage(void);
	const LocationBlock	*_findRuleBlock(void);

	friend std::ostream &operator<<(std::ostream &os, const Client &client);

public:
	Client(const fd_t idSocket, const fd_t requestSocket, Server const &server, const struct sockaddr_in &addr);
	Client(const Client &other);

	~Client(void);

	Client	&operator=(const Client &other);

	// error_t		init(const fd_t idSocket, const fd_t requestSocket, Server const *server);
	error_t		init(void);
	error_t		handleIn(fd_t fd);
	error_t		handleOut(fd_t fd);

	// GETTERS
	const RequestContext_t		&context(void) const;
	const struct sockaddr_in	&addr(void) const;
	time_t						timestamp(void) const;
	void						sockets(fd_t fds[2]) const;

	// SETTERS
	static void	setEpollFd(const int32_t fd);
};

#endif /* ******************************************************************* */
