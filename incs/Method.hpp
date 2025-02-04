#ifndef METHOD_HPP
#define METHOD_HPP

#include <string>
#include <vector>

typedef enum {
	METHOD_GET,
	METHOD_POST,
	METHOD_DELETE,
	METHOD_PUT,
	METHOD_HEAD,
	METHOD_INVAL_METHOD,
	METHOD_UNDEFINED = -1
} method_t;

class Method {
public:
	Method(void);
	Method(const Method &other);
	Method(const std::string &str);
	~Method(void);

	Method &operator=(const Method &other);

	bool operator==(const Method &other);
	bool operator==(const method_t method);
	bool operator!=(const Method &other);
	bool operator!=(const method_t method);

	method_t           index(void) const;
	const std::string &string(void) const;

	bool isValid(void) const;

	static const std::vector<std::string> methods;

private:
	method_t _index;

	static std::vector<std::string> _initMethods(void);
};

#endif /* ******************************************************************* */
