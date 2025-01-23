#ifndef ENV_HPP
#define ENV_HPP

#include <string>
#include <vector>

#include "RequestContext.hpp"

class Env {
public:
	Env(void);
	Env(const Env &other);
	Env(const RequestContext_t &context);
	~Env(void);

	Env &operator=(const Env &other);

	void   add(const std::string &key, const std::string &val);
	char **envp(void) const;

	static void destroy(char **envp);

private:
	std::vector<std::string> _envars;

	void _addContext(const RequestContext_t &context);
	void _addHeaders(const headers_t &headers);

	friend std::ostream &operator<<(std::ostream &os, const Env &path);
};

#endif /* ******************************************************************* */
