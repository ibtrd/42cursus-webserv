#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "webdef.hpp"
# include "Response.hpp"
# include <string>
# include <map>

# define REQ_BUFFER_SIZE 1024

class Request {
private:
	static	char 	_readBuffer[REQ_BUFFER_SIZE];
	static	int32_t _epollFd;

	bool			_readComplete;
	bool			_canWrite;
	
	int32_t			_socket;
	// Truc			&_truc;		// Server rules (reference or pointer)
	std::string		_buffer;

	Method			_method;
	std::string		_url;	
	std::string		_protocolVersion;

	std::map<std::string, std::string>	_headers;
	std::string							_body;

	Response		_response;
	std::string		_responseBuffer;

public:
	Request(void);
	Request(const Request &other);

	~Request(void);

	Request	&operator=(const Request &other);

	error_t		init(const int32_t requestSocket);
	error_t		handle(void);
	error_t		readSocket(void);
	StatusCode	parseRequestLine(void);
	error_t		switchToWrite(void);
	error_t		sendResponse(void);

	int32_t	socket(void) const;

	static int32_t	epollFd(void);

	static void	setEpollFd(const int32_t fd);
};

#endif /* ******************************************************************* */