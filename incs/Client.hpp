#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webdef.hpp"
# include "ARequest.hpp"
# include "Response.hpp"

# include <string>
# include <map>

class Client {
private:
	static	char 	_readBuffer[REQ_BUFFER_SIZE];
	static	int32_t _epollFd;
	static	ARequest	*(*_requestsBuilder[INVAL_METHOD])(Client &client);

	ARequest		*_request;
	uint32_t		_requestState;	// will replace _readComplete, _writeComplete, _canWrite and more
	
	int32_t			_socket;
	// Truc			&_truc;		// Server rules (reference or pointer)
	std::string		_buffer;

	Method			_method;
	std::string		_target;	
	std::string		_protocolVersion;

	std::map<std::string, std::string>	_headers;

	// std::string		_body;

	Response		_response;
	std::string		_responseBuffer;

	const std::string	_requestStateStr(void) const;

	error_t		_readSocket(void);
	error_t		_parseRequest(void);
	error_t		_parseRequestLine(void);
	error_t		_parseHeaders(void);

public:
	Client(void);
	Client(const Client &other);

	~Client(void);

	Client	&operator=(const Client &other);

	error_t		init(const int32_t requestSocket);
	error_t		handle(void);
	error_t		switchToWrite(void);
	error_t		sendResponse(void);

	// GETTERS

	// SETTERS
	static void	setEpollFd(const int32_t fd);
};

#endif /* ******************************************************************* */
