#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

# include <netinet/in.h>

# include "LocationBlock.hpp"

class ServerBlock {
public:
	ServerBlock(void);
	ServerBlock(const ServerBlock &other);

	~ServerBlock(void);

	ServerBlock	&operator=(const ServerBlock &other);

	// GETTERS
	const std::vector<struct sockaddr_in>	&hosts(void) const;
	const std::vector<std::string>			&names(void) const;
	const std::vector<LocationBlock>		&locations(void) const;

	const LocationBlock *findLocationBlock(const Path &target) const;

	// SETTERS
	void	addHost(const struct sockaddr_in &host);
	void	addName(const std::string &name);
	void	addLocation(const LocationBlock &location);

private:
	std::vector<struct sockaddr_in>	_hosts;
	std::vector<std::string>		_names;
	std::vector<LocationBlock>		_locations;
};

typedef std::map<fd_t, std::vector<ServerBlock> > servermap_t;

#endif /* ******************************************************************* */
