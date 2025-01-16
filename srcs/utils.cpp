// function a ranger quand on aura le temps (si nécessaire)

#include <sstream>

#include "webdef.hpp"

std::string statusCodeToReason(const status_code_t code) {
	switch (code) {
		case STATUS_OK:
			return ("STATUS_OK");
		case STATUS_CREATED:
			return ("Created");
		case STATUS_BAD_REQUEST:
			return ("Bad Request");
		case STATUS_UNAUTHORIZED:
			return ("Unauthorized");
		case STATUS_FORBIDDEN:
			return ("Forbidden");
		case STATUS_NOT_FOUND:
			return ("Not Found");
		case STATUS_METHOD_NOT_ALLOWED:
			return ("Method Not Allowed");
		case STATUS_REQUEST_TIMEOUT:
			return ("Request Timeout");
		case STATUS_CONFLICT:
			return ("Conflict");
		case STATUS_LENGTH_REQUIRED:
			return ("Length Required");
		case STATUS_PAYLOAD_TOO_LARGE:
			return ("Payload Too Large");
		case STATUS_URI_TOO_LONG:
			return ("URI Too Long");
		case STATUS_UNSUPPORTED_MEDIA_TYPE:
			return ("Unsupported Media Type");
		case STATUS_I_AM_A_TEAPOT:
			return ("I'm a teapot");
		case STATUS_TOO_MANY_REQUESTS:
			return ("Too Many Requests");
		case STATUS_INTERNAL_SERVER_ERROR:
			return ("Internal Server Error");
		case STATUS_NOT_IMPLEMENTED:
			return ("Not Implemented");
		case STATUS_SERVICE_UNAVAILABLE:
			return ("Service Unavailable");
		case STATUS_HTTP_VERSION_NOT_SUPPORTED:
			return ("HTTP Version Not Supported");
		default:
			return ("Invalid Status Code");
	}
}
