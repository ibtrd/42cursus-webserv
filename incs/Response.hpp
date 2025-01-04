#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "webdef.hpp"
# include <string>
# include <map>

# define REQ_BUFFER_SIZE 1024

class Response {
private:
	StatusCode	_statusCode;
	std::string	_reasonPhrase;
	// std::string	_statusLine;
	std::map<std::string, std::string>	_headers;
	std::string	_body;

public:
	Response(void);
	Response(const Response &other);

	~Response(void);

	Response	&operator=(const Response &other);

	StatusCode	statusCode(void) const;
	std::string	reasonPhrase(void) const;
	std::string statusLine(void) const;
	std::string header(const std::string &key) const;
	std::string body(void) const;
	std::string	response(void) const;

	void		setStatusCode(const StatusCode &code);
	void		setHeader(const std::string &key, const std::string &value);
	void		setBody(const std::string &body);

};

#endif /* ******************************************************************* */
