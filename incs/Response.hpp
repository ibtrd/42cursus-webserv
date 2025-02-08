#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webdef.hpp"

class Response {
private:
	status_code_t _statusCode;
	std::string   _reasonPhrase;
	headers_t     _headers;
	std::string   _body;

public:
	Response(void);
	Response(const Response &other);

	~Response(void);

	Response &operator=(const Response &other);

	status_code_t             statusCode(void) const;
	std::string               reasonPhrase(void) const;
	std::string               statusLine(void) const;
	headers_t::iterator       header(const std::string &key);
	std::string               body(void) const;
	std::string               response(void) const;
	size_t                    bodySize(void) const;
	headers_t::const_iterator headersBegin(void) const;
	headers_t::const_iterator headersEnd(void) const;

	void setStatusCode(const status_code_t &code);
	void setHeader(const std::string &key, const std::string &value);
	void setReasonPhrase(const std::string &reasonPhrase);
	void setBody(const std::string &body);
	void addBody(const std::string &body);

	void deleteHeader(const std::string &key);

	void clearBody(void);
	void clearHeaders(void);
	void clear(void);
};

#endif /* ******************************************************************* */
