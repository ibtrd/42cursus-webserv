#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include <stdint.h>
# include <string>
# include <exception>
# include <vector>
# include <map>

# include "ServerBlock.hpp"

# define DEFAULT_CONF_FILEPATH "./conf/default.conf"
# define DEFAULT_BACKLOG 511

class Configuration {	
public:
	Configuration(void);
	Configuration(const Configuration &other);
	Configuration(int argc, char *argv[]);

	~Configuration(void);

	Configuration	&operator=(const Configuration &other);

	// GETTERS
	int32_t 			backlog(void) const;

	// SETTERS
	void	setBacklog(const int32_t backlog);

private:

	int32_t						_backlog;
	std::vector<ServerBlock>	_blocks;

	void	_parseFile(const char *filepath);

	void	_serverDirective(void);


	typedef std::map<std::string, void (Configuration::*)()> DirectiveMap;

	static const DirectiveMap	directives;

	static DirectiveMap	_initializeDirectives(void);
};

#endif /* ******************************************************************* */
