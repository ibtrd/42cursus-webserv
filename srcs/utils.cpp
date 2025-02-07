#include "RequestContext.hpp"
#include "ft.hpp"
#include "webservHTML.hpp"

std::string statusCodeToReason(const status_code_t code) {
	switch (code) {
		case STATUS_OK:
			return ("OK");
		case STATUS_CREATED:
			return ("Created");
		case STATUS_NO_CONTENT:
			return ("No content");
		case STATUS_MULTIPLE_CHOICES:
			return ("Multiple Choices");
		case STATUS_MOVED_PERMANENTLY:
			return ("Moved Permanently");
		case STATUS_FOUND:
			return ("Found");
		case STATUS_SEE_OTHER:
			return ("See Other");
		case STATUS_NOT_MODIFIED:
			return ("Not Modified");
		case STATUS_TEMPORARY_REDIRECT:
			return ("Temporary Redirect");
		case STATUS_PERMANENT_REDIRECT:
			return ("Permanent Redirect");
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
		case STATUS_GATEWAY_TIMEOUT:
			return ("Gateway Timeout");
		case STATUS_HTTP_VERSION_NOT_SUPPORTED:
			return ("HTTP Version Not Supported");
		default:
			return ("Invalid Status Code");
	}
}

std::string statusCodeToMsg(const status_code_t code) {
	switch (code) {
		case STATUS_BAD_REQUEST:
			return (ERROR400MSG);
		case STATUS_UNAUTHORIZED:
			return (ERROR401MSG);
		case STATUS_FORBIDDEN:
			return (ERROR403MSG);
		case STATUS_NOT_FOUND:
			return (ERROR404MSG);
		case STATUS_METHOD_NOT_ALLOWED:
			return (ERROR405MSG);
		case STATUS_REQUEST_TIMEOUT:
			return (ERROR408MSG);
		case STATUS_CONFLICT:
			return (ERROR409MSG);
		case STATUS_LENGTH_REQUIRED:
			return (ERROR411MSG);
		case STATUS_PAYLOAD_TOO_LARGE:
			return (ERROR413MSG);
		case STATUS_URI_TOO_LONG:
			return (ERROR414MSG);
		case STATUS_UNSUPPORTED_MEDIA_TYPE:
			return (ERROR415MSG);
		case STATUS_I_AM_A_TEAPOT:
			return (ERROR418MSG);
		case STATUS_TOO_MANY_REQUESTS:
			return (ERROR429MSG);
		case STATUS_INTERNAL_SERVER_ERROR:
			return (ERROR500MSG);
		case STATUS_NOT_IMPLEMENTED:
			return (ERROR501MSG);
		case STATUS_SERVICE_UNAVAILABLE:
			return (ERROR503MSG);
		case STATUS_GATEWAY_TIMEOUT:
			return (ERROR504MSG);
		case STATUS_HTTP_VERSION_NOT_SUPPORTED:
			return (ERROR505MSG);
		default:
			return ("Invalid Status Code");
	}
}

int32_t sToContentLength(const std::string &str, bool chunked) {
	char *endptr;
	errno = 0;
	long value;
	if (chunked) {
		value = std::strtol(str.c_str(), &endptr, 16);
	} else {
		value = std::strtol(str.c_str(), &endptr, 10);
	}

	if (*endptr != '\0' || value < 0) {
		return CONTENT_LENGTH_INVALID;
	} else if (value > std::numeric_limits<int32_t>::max()) {
		return CONTENT_LENGTH_TOO_LARGE;
	}
	return static_cast<int32_t>(value);
}
