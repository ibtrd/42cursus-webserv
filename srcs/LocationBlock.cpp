#include "LocationBlock.hpp"

/* CONSTRUCTORS ************************************************************* */

LocationBlock::LocationBlock(void) {
	this->_allowedMethods = DEFAULT_METHODS;
	this->_dirListing = DEFAULT_DIRLISTING;
	this->_maxBodySize = DEFAULT_MAXBODYSIZE;
}

LocationBlock::LocationBlock(const LocationBlock &other) {
	*this = other;
}

LocationBlock::LocationBlock(const std::string &path) {
	this->_path = path;
	this->_allowedMethods = DEFAULT_METHODS;
	this->_dirListing = DEFAULT_DIRLISTING;
	this->_maxBodySize = DEFAULT_MAXBODYSIZE;
}

LocationBlock::~LocationBlock(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

LocationBlock	&LocationBlock::operator=(const LocationBlock &other) {
	if (this == &other)
		return (*this);
	this->_path = other._path;
	this->_dirListing = other._dirListing;
	this->_maxBodySize = other._maxBodySize;
	this->_root = other._root;
	this->_allowedMethods = other._allowedMethods;
	return (*this);
}

/* ************************************************************************** */

/* SETTERS ****************************************************************** */

bool LocationBlock::allowMethod(const std::string &str) {
	const std::vector<std::string> &methods = LocationBlock::_methods;

	for (std::size_t i = 0; i < methods.size(); ++i) {
		if (0 == methods[i].compare(str)) {
			this->_allowedMethods |= (1 << i);
			return true;
		}
	}
	return false;
}

bool	LocationBlock::setDirListing(const std::string &str) {
	if (0 == str.compare("on")) {
		this->_dirListing = true;
		return true;
	} else if (0 == str.compare("off")) {
		this->_dirListing = false;
		return true;
	}
	return false;
}

void	LocationBlock::setMaxBodySize(const int32_t size) {
	this->_maxBodySize = size;
}

void	LocationBlock::setRoot(const std::string &str) {
	this->_root = str;
}

/* GETTERS ****************************************************************** */

const std::string &LocationBlock::path(void) const {
	return this->_path;
}

bool LocationBlock::isDirListing(void) const {
	return this->_dirListing;
}

int32_t LocationBlock::getMaxBodySize(void) const {
	return this->_maxBodySize;
}

const std::string &LocationBlock::getRoot(void) const {
	return this->_root;
}

bool LocationBlock::isAllowedMethod(const std::string &str) const {
	const std::vector<std::string> &methods = LocationBlock::_methods;

	for (std::size_t i = 0; i < methods.size(); ++i) {
		if (0 == methods[i].compare(str)) {
			return this->_allowedMethods & (1 << i);
		}
	}
	return false;
}

/* STATICS ****************************************************************** */

const std::vector<std::string> LocationBlock::_methods = LocationBlock::_initMethods();

std::vector<std::string> LocationBlock::_initMethods(void) {
	std::vector<std::string>	methods;
	methods.push_back("GET");
	methods.push_back("POST");
	methods.push_back("DELETE");
	return methods;
}