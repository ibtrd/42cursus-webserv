#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include <netinet/in.h>

#include "LocationBlock.hpp"

typedef std::map<status_code_t, Path> error_pages_t;

class ServerBlock {
   public:
	ServerBlock(void);
	ServerBlock(const ServerBlock &other);

	~ServerBlock(void);

	ServerBlock &operator=(const ServerBlock &other);

	// GETTERS
	const std::vector<struct sockaddr_in> &hosts(void) const;
	const std::vector<std::string>        &names(void) const;
	const std::vector<LocationBlock>      &locations(void) const;
	const error_pages_t                   &errorPages(void) const;

	// SETTERS
	void addHost(const struct sockaddr_in &host);
	void addName(const std::string &name);
	void addLocation(const LocationBlock &location);
	void addErrorPage(const status_code_t code, const Path &file);

	const LocationBlock *findLocationBlock(const Path &target) const;
	const Path          *findErrorPage(status_code_t code) const;

	void fillLocations(const LocationBlock &location);

   private:
	std::vector<struct sockaddr_in> _hosts;
	std::vector<std::string>        _names;
	std::vector<LocationBlock>      _locations;
	error_pages_t                   _errorPages;
};

#endif /* ******************************************************************* */
