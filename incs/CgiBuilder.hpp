#ifndef CGIBUILDER_HPP
#define CGIBUILDER_HPP

#include <string>
#include <vector>

#include "ARequest.hpp"
#include "RequestContext.hpp"

class CgiBuilder {
public:
	CgiBuilder(void);
	CgiBuilder(const CgiBuilder &other);
	CgiBuilder(const ARequest *req);
	~CgiBuilder(void);

	CgiBuilder &operator=(const CgiBuilder &other);

	void   addEnvar(const std::string &key, const std::string &val);
	char **envp(void) const;
	char **argv(void) const;

	static void destroy(char **ptr);

private:
	std::vector<std::string> 	_envars;
	std::vector<std::string> 	_arguments;

	void _addContext(const RequestContext_t &context);
	void _addHeaders(const headers_t &headers);

	friend std::ostream &operator<<(std::ostream &os, const CgiBuilder &path);
};

#endif /* ******************************************************************* */
