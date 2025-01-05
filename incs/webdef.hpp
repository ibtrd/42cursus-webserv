#ifndef WEBDEF_HPP
# define WEBDEF_HPP

# include <stdint.h>
# include <string>
# include <map>
# include <vector>

# include "ServerBlock.hpp"

typedef int32_t error_t;
typedef int fd_t;
typedef std::map<fd_t, std::vector<ServerBlock> > servermap_t;
typedef std::map<fd_t, std::vector<struct sockaddr_in> > bindmap_t;

typedef enum {
	NONE = 0,
	OK = 200,
	CREATED = 201,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	REQUEST_TIMEOUT = 408,
	LENGTH_REQUIRED = 411,
	PAYLOAD_TOO_LARGE = 413,
	URI_TOO_LONG = 414,
	UNSUPPORTED_MEDIA_TYPE = 415,
	I_AM_A_TEAPOT = 418,
	TOO_MANY_REQUESTS = 429,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	SERVICE_UNAVAILABLE = 503,
	HTTP_VERSION_NOT_SUPPORTED = 505
} StatusCode;

typedef enum {
	GET,
	POST,
	DELETE,
	INVAL_METHOD
} Method;

// typedef enum {
// 	INVAL_HEADER,
// 	CONTENT_LENGTH,
// 	CONTENT_TYPE
// } Header;

Method		stringToMethod(const std::string &method);
std::string	methodToString(const Method method);
std::string	statusCodeToReason(const StatusCode code);

#endif
