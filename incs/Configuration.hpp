#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

#include <stdint.h>
#include <string>
#include <exception>

class Configuration {
private:
	int32_t		_port;
	int32_t		_backlog;
	
public:
	Configuration(void);
	Configuration(const Configuration &other);
	Configuration(int argc, char *argv[]);

	~Configuration(void);

	Configuration	&operator=(const Configuration &other);

	int32_t				port(void) const;
	int32_t 			backlog(void) const;
	const std::string	&name(void) const;

	void	setPort(const int32_t port);
	void	setBacklog(const int32_t backlog);
	void	setName(const std::string &name);

	class	BadArgumentsException : public std::runtime_error {};
};

#endif /* ******************************************************************* */
