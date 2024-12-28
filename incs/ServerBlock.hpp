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
	const std::vector<std::pair<in_addr_t, in_port_t> > &getHosts(void) const;
	const std::vector<std::string>						&getNames(void) const;
	const std::vector<LocationBlock>					&getLocations(void) const;

	// SETTERS
	void	addHost(const std::pair<in_addr_t, in_port_t> host);
	void	addName(const std::string &name);
	void	addLocation(const LocationBlock &location);

private:
	std::vector<std::pair<in_addr_t, in_port_t> >	_hosts;
	std::vector<std::string>						_names;
	std::vector<LocationBlock>						_locations;
};

#endif /* ******************************************************************* */
