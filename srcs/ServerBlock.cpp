#include "ServerBlock.hpp"

/* CONSTRUCTORS ************************************************************* */

ServerBlock::ServerBlock(void) {}

ServerBlock::ServerBlock(const ServerBlock &other) { *this = other; }

ServerBlock::~ServerBlock(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

ServerBlock &ServerBlock::operator=(const ServerBlock &other) {
	if (this == &other) {
		return (*this);
	}
	this->_hosts      = other._hosts;
	this->_names      = other._names;
	this->_locations  = other._locations;
	this->_errorPages = other._errorPages;
	return (*this);
}

/* ************************************************************************** */

/* GETTERS ****************************************************************** */

const std::vector<struct sockaddr_in> &ServerBlock::hosts(void) const { return this->_hosts; }

const std::vector<std::string> &ServerBlock::names(void) const { return this->_names; }

const std::vector<LocationBlock> &ServerBlock::locations(void) const { return this->_locations; }

const error_pages_t &ServerBlock::errorPages(void) const { return this->_errorPages; }

const LocationBlock *ServerBlock::findLocationBlock(const Path &target) const {
	const std::vector<LocationBlock> &locations = this->_locations;
	const LocationBlock              *selected  = NULL;
	uint32_t                          bestMatch = 0;

	for (uint32_t i = 0; i < locations.size(); ++i) {
		const Path &path = locations[i].path();
		if (path.length() > target.length()) {
			continue;
		}
		if (locations[i].match(target)) {
			if (path.prefixLength() == target.length()) {
				return &locations[i];
			} else if (path.prefixLength() > bestMatch || !selected) {
				bestMatch = path.prefixLength();
				selected  = &locations[i];
			}
		}
	}
	return selected;
}

const Path *ServerBlock::findErrorPage(status_code_t code) const {
	error_pages_t::const_iterator it = this->_errorPages.find(code);
	if (it != this->_errorPages.end()) {
		return &it->second;
	}
	return NULL;
}

void ServerBlock::fillLocations(const LocationBlock &location) {
	for (uint32_t i = 0; i < this->_locations.size(); ++i) {
		this->_locations[i].fill(location);
	}
}

/* SETTERS ****************************************************************** */

void ServerBlock::addHost(const struct sockaddr_in &host) { this->_hosts.push_back(host); }

void ServerBlock::addName(const std::string &name) { this->_names.push_back(name); }

void ServerBlock::addLocation(const LocationBlock &location) {
	this->_locations.push_back(location);
}

void ServerBlock::addErrorPage(const status_code_t code, const Path &file) {
	this->_errorPages[code] = file;
}
