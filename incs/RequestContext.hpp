#ifndef REQUESTCONTEXT_HPP
#define REQUESTCONTEXT_HPP

#include "BinaryBuffer.hpp"
#include "Queries.hpp"
#include "Response.hpp"
#include "ServerBlock.hpp"

#define REQ_BUFFER_SIZE 65535
#define REQ_DIR_BUFFER_SIZE 128

#define REQ_CONTINUE 0  // Incomplete task, skip to next call
#define REQ_DONE 1      // Done with current task
#define REQ_ERROR 2     // Program error

#define PARENT_SOCKET 0
#define CHILD_SOCKET 1

// Request state flags

// No state
#define REQ_STATE_NONE 0x00000000
// The request line has been read
#define REQ_STATE_READ_REQUEST_LINE_COMPLETE 0x00000001
// The headers have been read
#define REQ_STATE_READ_HEADERS_COMPLETE 0x00000002
// The client has finished reading
#define REQ_STATE_CLIENT_READ_COMPLETE 0x00000003
// The request has been read
#define REQ_STATE_READ_COMPLETE 0x00000007
// The proccessing on socket in is complete
#define REQ_STATE_WORK_IN_COMPLETE 0x00000010
// The proccessing on socket out is complete
#define REQ_STATE_WORK_OUT_COMPLETE 0x00000020
// The proccessing is complete
#define REQ_STATE_WORK_COMPLETE 0x00000030
// The response can be written
#define REQ_STATE_CAN_WRITE 0x00000040
// The response has been written
#define REQ_STATE_WRITE_COMPLETE 0x00000080

#define IS_REQ_READ_REQUEST_LINE_COMPLETE(x) \
	((x & REQ_STATE_READ_REQUEST_LINE_COMPLETE) == REQ_STATE_READ_REQUEST_LINE_COMPLETE)
#define IS_REQ_READ_HEADERS_COMPLETE(x) \
	((x & REQ_STATE_READ_HEADERS_COMPLETE) == REQ_STATE_READ_HEADERS_COMPLETE)
#define IS_REQ_CLIENT_READ_COMPLETE(x) \
	((x & REQ_STATE_CLIENT_READ_COMPLETE) == REQ_STATE_CLIENT_READ_COMPLETE)
#define IS_REQ_READ_COMPLETE(x) ((x & REQ_STATE_READ_COMPLETE) == REQ_STATE_READ_COMPLETE)
#define IS_REQ_WORK_IN_COMPLETE(x) ((x & REQ_STATE_WORK_IN_COMPLETE) == REQ_STATE_WORK_IN_COMPLETE)
#define IS_REQ_WORK_OUT_COMPLETE(x) \
	((x & REQ_STATE_WORK_OUT_COMPLETE) == REQ_STATE_WORK_OUT_COMPLETE)
#define IS_REQ_WORK_COMPLETE(x) ((x & REQ_STATE_WORK_COMPLETE) == REQ_STATE_WORK_COMPLETE)
#define IS_REQ_CAN_WRITE(x) ((x & REQ_STATE_CAN_WRITE) == REQ_STATE_CAN_WRITE)
#define IS_REQ_WRITE_COMPLETE(x) ((x & REQ_STATE_WRITE_COMPLETE) == REQ_STATE_WRITE_COMPLETE)

#define SET_REQ_READ_REQUEST_LINE_COMPLETE(x) (x |= REQ_STATE_READ_REQUEST_LINE_COMPLETE)
#define SET_REQ_READ_HEADERS_COMPLETE(x) (x |= REQ_STATE_READ_HEADERS_COMPLETE)
#define SET_REQ_CLIENT_READ_COMPLETE(x) (x |= REQ_STATE_CLIENT_READ_COMPLETE)
#define SET_REQ_READ_COMPLETE(x) (x |= REQ_STATE_READ_COMPLETE)
#define SET_REQ_WORK_IN_COMPLETE(x) (x |= REQ_STATE_WORK_IN_COMPLETE)
#define SET_REQ_WORK_OUT_COMPLETE(x) (x |= REQ_STATE_WORK_OUT_COMPLETE)
#define SET_REQ_WORK_COMPLETE(x) (x |= REQ_STATE_WORK_COMPLETE)
#define SET_REQ_CAN_WRITE(x) (x |= REQ_STATE_CAN_WRITE)
#define SET_REQ_WRITE_COMPLETE(x) (x |= REQ_STATE_WRITE_COMPLETE)

#define UNSET_REQ_WORK_OUT_COMPLETE(x) (x &= ~REQ_STATE_WORK_OUT_COMPLETE)

#define RETURN_UNLESS(ret, code) \
	if (ret != code) {           \
		return ret;              \
	}

#define HEADER_HOST "Host"
#define HEADER_CONTENT_LENGTH "Content-Length"
#define HEADER_CONTENT_TYPE "Content-Type"
#define HEADER_USER_AGENT "User-Agent"
#define HEADER_ACCEPT_ENCODING "Accept-Encoding"
#define HEADER_LOCATION "Location"
#define HEADER_TRANSFER_ENCODING "Transfer-Encoding"
#define HEADER_SERVER "Server"

#define CONTENT_LENGTH_INVALID -2
#define CONTENT_LENGTH_TOO_LARGE -1

#define HEADER_TRANSFER_CHUNKE "chunked"

class Server;

typedef struct RequestContext_s {
	Server              &server;
	const ServerBlock   *serverBlock;
	const LocationBlock *ruleBlock;
	struct sockaddr_in  addr;

	fd_t _cgiSockets[2];

	uint32_t     requestState;
	BinaryBuffer buffer;

	Method      method;
	std::string target;
	std::string protocolVersion;

	Queries queries;

	headers_t headers;

	Response     response;
	BinaryBuffer responseBuffer;

	RequestContext_s(Server &server, const struct sockaddr_in &addr)
	    : server(server), serverBlock(NULL), ruleBlock(NULL), addr(addr) {}

	RequestContext_s &operator=(const RequestContext_s &other) {
		if (this == &other) {
			return (*this);
		}
		this->serverBlock   = other.serverBlock;
		this->ruleBlock     = other.ruleBlock;

		this->addr = other.addr;
		// this->addr.sin_addr   = other.addr.sin_addr;
		// this->addr.sin_addr.s_addr = other.addr.sin_addr.s_addr;

		// this->addr.sin_family = other.addr.sin_family;
		// this->addr.sin_family.
		// this->addr.sin_port   = other.addr.sin_port;
		// this->addr.sin_zero   = other.addr.sin_zero;

		this->_cgiSockets[PARENT_SOCKET] = other._cgiSockets[PARENT_SOCKET];
		this->_cgiSockets[CHILD_SOCKET] = other._cgiSockets[CHILD_SOCKET];

		this->requestState  = other.requestState;
		this->buffer        = other.buffer;

		this->method        = other.method;
		this->target        = other.target;
		this->protocolVersion = other.protocolVersion;

		this->queries       = other.queries;
		this->headers       = other.headers;

		this->response      = other.response;
		this->responseBuffer = other.responseBuffer;
		return (*this);
	}

} RequestContext_t;


#endif
