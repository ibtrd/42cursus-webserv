#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webdef.hpp"
# include "Response.hpp"
# include "ARequest.hpp"
# include <string>
# include <map>

# define REQ_BUFFER_SIZE 1024

# define REQ_CONTINUE 0	// Client not fully received
# define REQ_DONE 1		// Client fully received
# define REQ_ERROR 2	// Program error

# define REQ_STATE_NONE							0x00000000
# define REQ_STATE_READ_REQUEST_LINE_COMPLETE	0x00000001
# define REQ_STATE_READ_HEADERS_COMPLETE		0x00000002
# define REQ_STATE_READ_BODY_COMPLETE			0x00000004
# define REQ_STATE_CLIENT_READ_COMPLETE			0x00000008
# define REQ_STATE_READ_COMPLETE				0x0000000F
# define REQ_STATE_CAN_WRITE					0x00000020
# define REQ_STATE_WRITE_COMPLETE				0x00000040

# define IS_REQ_READ_REQUEST_LINE_COMPLETE(x)	((x & REQ_STATE_READ_REQUEST_LINE_COMPLETE) == REQ_STATE_READ_REQUEST_LINE_COMPLETE)
# define IS_REQ_READ_HEADERS_COMPLETE(x)		((x & REQ_STATE_READ_HEADERS_COMPLETE) == REQ_STATE_READ_HEADERS_COMPLETE)
# define IS_REQ_READ_BODY_COMPLETE(x)			((x & REQ_STATE_READ_BODY_COMPLETE) == REQ_STATE_READ_BODY_COMPLETE)
# define IS_REQ_CLIENT_READ_COMPLETE(x)			((x & REQ_STATE_CLIENT_READ_COMPLETE) == REQ_STATE_CLIENT_READ_COMPLETE)
# define IS_REQ_READ_COMPLETE(x)				((x & REQ_STATE_READ_COMPLETE) == REQ_STATE_READ_COMPLETE)
# define IS_REQ_CAN_WRITE(x)					((x & REQ_STATE_CAN_WRITE) == REQ_STATE_CAN_WRITE)
# define IS_REQ_WRITE_COMPLETE(x)				((x & REQ_STATE_WRITE_COMPLETE) == REQ_STATE_WRITE_COMPLETE)

# define SET_REQ_READ_REQUEST_LINE_COMPLETE(x)	(x |= REQ_STATE_READ_REQUEST_LINE_COMPLETE)
# define SET_REQ_READ_HEADERS_COMPLETE(x)		(x |= REQ_STATE_READ_HEADERS_COMPLETE)
# define SET_REQ_READ_BODY_COMPLETE(x)			(x |= REQ_STATE_READ_BODY_COMPLETE)
# define SET_REQ_CLIENT_READ_COMPLETE(x)		(x |= REQ_STATE_CLIENT_READ_COMPLETE)
# define SET_REQ_READ_COMPLETE(x)				(x |= REQ_STATE_READ_COMPLETE)
# define SET_REQ_CAN_WRITE(x)					(x |= REQ_STATE_CAN_WRITE)
# define SET_REQ_WRITE_COMPLETE(x)				(x |= REQ_STATE_WRITE_COMPLETE)

class Client {
private:
	static	char 	_readBuffer[REQ_BUFFER_SIZE];
	static	int32_t _epollFd;
	static	ARequest	*(*_requestsBuilder[INVAL_METHOD])(void);

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

	// GETTERS

	// SETTERS
	static void	setEpollFd(const int32_t fd);
};

#endif /* ******************************************************************* */
