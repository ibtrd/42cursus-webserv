#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <stdint.h>
# include <string>
# include <map>

# define REQ_BUFFER_SIZE 1024

typedef enum {
	GET,
	POST,
	DELETE,
	INVAL_METHOD
} Method;

class Request {
private:
	static	char 	_readBuffer[REQ_BUFFER_SIZE];
	static	int32_t _epollFd;
	
	int32_t			_socket;
	std::string		_buffer;

	Method			_method;
	std::string		_url;	
	std::string		_protocolVersion;

	std::map<std::string, std::string>	_headers;
	std::string							_body;

public:
	Request(void);
	Request(const int32_t serverSocket);
	Request(const Request &other);

	~Request(void);

	Request	&operator=(const Request &other);

	int32_t	socket(void) const;

	static int32_t	epollFd(void);

	static void	setEpollFd(const int32_t fd);
};

#endif /* ******************************************************************* */
