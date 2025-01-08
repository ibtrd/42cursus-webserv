#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ARequest.hpp"

# include <string>
# include <map>

class Client {
private:
	static	char		_readBuffer[REQ_BUFFER_SIZE];
	static	int32_t		_epollFd;
	static	ARequest	*(*_requestsBuilder[INVAL_METHOD])(RequestContext_t &);
	
	int32_t			_socket;
	// Truc			&_truc;		// Server rules (reference or pointer)

	ARequest			*_request;
	RequestContext_t	_context;

	const std::string	_requestStateStr(void) const;

	error_t	_readSocket(void);
	error_t	_parseRequest(void);
	error_t	_parseRequestLine(void);
	error_t	_parseHeaders(void);
	error_t	_process(void);			// private
	error_t	_switchToWrite(void);	// private
	error_t	_sendResponse(void);	// private

public:
	Client(void);
	Client(const Client &other);

	~Client(void);

	Client	&operator=(const Client &other);

	error_t		init(const int32_t requestSocket);
	error_t		handle(void);

	// GETTERS

	// SETTERS
	static void	setEpollFd(const int32_t fd);
};

#endif /* ******************************************************************* */
