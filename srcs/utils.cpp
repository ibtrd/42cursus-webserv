// function a ranger quand on aura le temps (si nécessaire)

#include "webdef.hpp"

#include <sstream>

Method		stringToMethod(const std::string &method)
{
	if (method == "GET")
		return (GET);
	if (method == "POST")
		return (POST);
	if (method == "DELETE")
		return (DELETE);
	return (INVAL_METHOD);
}

std::string		methodToString(const Method method)
{
	switch (method)
	{
		case GET:
			return ("GET");
		case POST:
			return ("POST");
		case DELETE:
			return ("DELETE");
		default:
			return ("INVALID METHOD");
	}
}

std::string		statusCodeToReason(const StatusCode code)
{
	switch (code)
	{
		case OK:
			return ("OK");
		case CREATED:
			return ("Created");
		case BAD_REQUEST:
			return ("Bad Request");
		case UNAUTHORIZED:
			return ("Unauthorized");
		case FORBIDDEN:
			return ("Forbidden");
		case NOT_FOUND:
			return ("Not Found");
		case METHOD_NOT_ALLOWED:
			return ("Method Not Allowed");
		case REQUEST_TIMEOUT:
			return ("Request Timeout");
		case LENGTH_REQUIRED:
			return ("Length Required");
		case PAYLOAD_TOO_LARGE:
			return ("Payload Too Large");
		case URI_TOO_LONG:
			return ("URI Too Long");
		case UNSUPPORTED_MEDIA_TYPE:
			return ("Unsupported Media Type");
		case I_AM_A_TEAPOT:
			return ("I'm a teapot");
		case TOO_MANY_REQUESTS:
			return ("Too Many Requests");
		case INTERNAL_SERVER_ERROR:
			return ("Internal Server Error");
		case NOT_IMPLEMENTED:
			return ("Not Implemented");
		case SERVICE_UNAVAILABLE:
			return ("Service Unavailable");
		case HTTP_VERSION_NOT_SUPPORTED:
			return ("HTTP Version Not Supported");
		default:
			return ("Invalid Status Code");
	}
}
