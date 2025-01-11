#ifndef REQUESTCONTEXT
# define REQUESTCONTEXT

# include "Response.hpp"
# include "LocationBlock.hpp"

# define REQ_BUFFER_SIZE 1024

# define REQ_CONTINUE 0	// Incomplete task, skip to next call
# define REQ_DONE 1		// Done with current task
# define REQ_ERROR 2	// Program error

// Request state flags

// No state
# define REQ_STATE_NONE							0x00000000
// The request line has been read
# define REQ_STATE_READ_REQUEST_LINE_COMPLETE	0x00000001
// The headers have been read
# define REQ_STATE_READ_HEADERS_COMPLETE		0x00000002
// The client has finished reading
# define REQ_STATE_CLIENT_READ_COMPLETE			0x00000003
// The body has been read
# define REQ_STATE_READ_BODY_COMPLETE			0x00000004
// The request has been read
# define REQ_STATE_READ_COMPLETE				0x00000007
// The request has been processed
# define REQ_STATE_PROCESS_COMPLETE				0x00000010
// The response can be written
# define REQ_STATE_CAN_WRITE					0x00000020
// The response has been written
# define REQ_STATE_WRITE_COMPLETE				0x00000040

# define IS_REQ_READ_REQUEST_LINE_COMPLETE(x)	((x & REQ_STATE_READ_REQUEST_LINE_COMPLETE) == REQ_STATE_READ_REQUEST_LINE_COMPLETE)
# define IS_REQ_READ_HEADERS_COMPLETE(x)		((x & REQ_STATE_READ_HEADERS_COMPLETE) == REQ_STATE_READ_HEADERS_COMPLETE)
# define IS_REQ_CLIENT_READ_COMPLETE(x)			((x & REQ_STATE_CLIENT_READ_COMPLETE) == REQ_STATE_CLIENT_READ_COMPLETE)
# define IS_REQ_READ_BODY_COMPLETE(x)			((x & REQ_STATE_READ_BODY_COMPLETE) == REQ_STATE_READ_BODY_COMPLETE)
# define IS_REQ_READ_COMPLETE(x)				((x & REQ_STATE_READ_COMPLETE) == REQ_STATE_READ_COMPLETE)
# define IS_REQ_PROCESS_COMPLETE(x)				((x & REQ_STATE_PROCESS_COMPLETE) == REQ_STATE_PROCESS_COMPLETE)
# define IS_REQ_CAN_WRITE(x)					((x & REQ_STATE_CAN_WRITE) == REQ_STATE_CAN_WRITE)
# define IS_REQ_WRITE_COMPLETE(x)				((x & REQ_STATE_WRITE_COMPLETE) == REQ_STATE_WRITE_COMPLETE)

# define SET_REQ_READ_REQUEST_LINE_COMPLETE(x)	(x |= REQ_STATE_READ_REQUEST_LINE_COMPLETE)
# define SET_REQ_READ_HEADERS_COMPLETE(x)		(x |= REQ_STATE_READ_HEADERS_COMPLETE)
# define SET_REQ_READ_BODY_COMPLETE(x)			(x |= REQ_STATE_READ_BODY_COMPLETE)
# define SET_REQ_CLIENT_READ_COMPLETE(x)		(x |= REQ_STATE_CLIENT_READ_COMPLETE)
# define SET_REQ_READ_COMPLETE(x)				(x |= REQ_STATE_READ_COMPLETE)
# define SET_REQ_PROCESS_COMPLETE(x)			(x |= REQ_STATE_PROCESS_COMPLETE)
# define SET_REQ_CAN_WRITE(x)					(x |= REQ_STATE_CAN_WRITE)
# define SET_REQ_WRITE_COMPLETE(x)				(x |= REQ_STATE_WRITE_COMPLETE)

class Server;

typedef struct RequestContext_s {
	Server const		*server;
	LocationBlock const *ruleBlock;

	uint32_t		requestState;
	std::string		buffer;

	Method			method;
	std::string		target;	
	std::string		protocolVersion;

	headers_t		headers;

	Response		response;
	std::string		responseBuffer;
} RequestContext_t;

#endif
