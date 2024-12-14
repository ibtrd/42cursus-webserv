#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "webdef.hpp"
# include <string>
# include <map>

# define REQ_BUFFER_SIZE 1024

class Request {
private:
	static	char 	_readBuffer[REQ_BUFFER_SIZE];
	static	int32_t _epollFd;

	bool			_readComplete;
	
	int32_t			_socket;
	std::string		_buffer;

	Method			_method;
	std::string		_url;	
	std::string		_protocolVersion;

	std::map<std::string, std::string>	_headers;
	std::string							_body;

	response_t	_response;

public:
	Request(void);
	Request(const Request &other);

	~Request(void);

	Request	&operator=(const Request &other);

	error_t		initRequest(const int32_t serverSocket);
	error_t		handleRequest(void);
	error_t		readSocket(void);
	status_t	parseRequestLine(void);
	error_t		sendResponse(void);
	error_t		generateResponse(const StatusCode code, const std::string *body);

	int32_t	socket(void) const;

	static int32_t	epollFd(void);

	static void	setEpollFd(const int32_t fd);
};

#endif /* ******************************************************************* */
