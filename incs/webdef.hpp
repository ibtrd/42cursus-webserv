#ifndef WEBDEF_HPP
#define WEBDEF_HPP

#include <stdint.h>

#include <map>
#include <string>

typedef int32_t error_t;
typedef int     fd_t;

typedef std::map<std::string, std::string> headers_t;
typedef std::map<std::string, std::string> mimetypes_t;

typedef enum {
	STATUS_NONE                       = 0,
	STATUS_OK                         = 200,
	STATUS_CREATED                    = 201,
	STATUS_NO_CONTENT                 = 204,
	STATUS_MULTIPLE_CHOICES           = 300,
	STATUS_MOVED_PERMANENTLY          = 301,
	STATUS_FOUND                      = 302,
	STATUS_SEE_OTHER                  = 303,
	STATUS_NOT_MODIFIED               = 304,
	STATUS_TEMPORARY_REDIRECT         = 307,
	STATUS_PERMANENT_REDIRECT         = 308,
	STATUS_BAD_REQUEST                = 400,
	STATUS_UNAUTHORIZED               = 401,
	STATUS_FORBIDDEN                  = 403,
	STATUS_NOT_FOUND                  = 404,
	STATUS_METHOD_NOT_ALLOWED         = 405,
	STATUS_REQUEST_TIMEOUT            = 408,
	STATUS_CONFLICT                   = 409,
	STATUS_LENGTH_REQUIRED            = 411,
	STATUS_PAYLOAD_TOO_LARGE          = 413,
	STATUS_URI_TOO_LONG               = 414,
	STATUS_UNSUPPORTED_MEDIA_TYPE     = 415,
	STATUS_I_AM_A_TEAPOT              = 418,
	STATUS_TOO_MANY_REQUESTS          = 429,
	STATUS_INTERNAL_SERVER_ERROR      = 500,
	STATUS_NOT_IMPLEMENTED            = 501,
	STATUS_SERVICE_UNAVAILABLE        = 503,
	STATUS_HTTP_VERSION_NOT_SUPPORTED = 505
} StatusCode;

typedef uint16_t status_code_t;

enum timeout { CLIENT_HEADER_TIMEOUT, CLIENT_BODY_TIMEOUT, SEND_TIMEOUT, TIMEOUT_COUNT };

std::string statusCodeToReason(const status_code_t code);
std::string statusCodeToMsg(const status_code_t code);
int32_t		sToContentLength(const std::string &str);

#endif
