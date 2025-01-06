#include "ServerBlock.hpp"

/* CONSTRUCTORS ************************************************************* */

ServerBlock::ServerBlock(void) {}

ServerBlock::ServerBlock(const ServerBlock &other) {
	*this = other;
}

ServerBlock::~ServerBlock(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

ServerBlock	&ServerBlock::operator=(const ServerBlock &other) {
	if (this == &other)
		return (*this);
	this->_hosts = other._hosts;
	this->_names = other._names;
	this->_locations = other._locations;
	return (*this);
}

/* ************************************************************************** */

/* GETTERS ****************************************************************** */

const std::vector<struct sockaddr_in> &ServerBlock::hosts(void) const {
	return this->_hosts;
}

const std::vector<std::string> &ServerBlock::names(void) const {
	return this->_names;
}

const std::vector<LocationBlock> &ServerBlock::locations(void) const {
	return this->_locations;
}

const LocationBlock &ServerBlock::fetchLocationBlock(const Path &target) const {
	const std::vector<LocationBlock>	&locations = this->_locations;
	const LocationBlock						*selected = NULL;
	int32_t							best = 0;

	for (uint32_t i = 0; i < locations.size(); ++i) {
		const int32_t match = locations[i].match(target);
		if (match > best) {
			best = match;
			selected = &locations[i];
		}
	}

	if (!selected)
		throw std::string("404");
	return *selected;
}

/* SETTERS ****************************************************************** */

void ServerBlock::addHost(const struct sockaddr_in &host) {
	this->_hosts.push_back(host);
}

void ServerBlock::addName(const std::string &name) {
	this->_names.push_back(name);
}

void ServerBlock::addLocation(const LocationBlock &location) {
	this->_locations.push_back(location);
}

/* EXCEPTIONS *************************************************************** */
