#ifndef METHOD_HPP
#define METHOD_HPP

#include <string>
#include <vector>

typedef enum { GET, POST, DELETE, PUT, INVAL_METHOD } method_t;

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
