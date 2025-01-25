#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webdef.hpp"

class Response {
private:
	status_code_t _statusCode;
	std::string   _reasonPhrase;
	headers_t     _headers;
	// DEPRECATED
	std::string _body;

	bool _isCgi;

public:
	Response(void);
	Response(const Response &other);

	~Response(void);

	Response &operator=(const Response &other);

	status_code_t statusCode(void) const;
	std::string   reasonPhrase(void) const;
	std::string   statusLine(void) const;
	std::string   header(const std::string &key) const;
	std::string   body(void) const;
	std::string   response(void) const;
	size_t        bodySize(void) const;

	void setStatusCode(const status_code_t &code);
	void setHeader(const std::string &key, const std::string &value);
	// DEPRECATED
	void setBody(const std::string &body);
	void addBody(const std::string &body);
	void clearBody(void);

	void enableIsCgi(void);
	void diableIsCgi(void);
};

#endif /* ******************************************************************* */
