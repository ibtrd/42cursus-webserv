#include <stdexcept>

#include "LocationBlock.hpp"
#include "webdef.hpp"

/* CONSTRUCTORS ************************************************************* */

LocationBlock::LocationBlock(void) {
	this->_allowed = DEFAULT_METHODS;
	this->_dirListing = DEFAULT_DIRLISTING;
	this->_maxBodySize = DEFAULT_MAXBODYSIZE;
}

LocationBlock::LocationBlock(const LocationBlock &other) {
	*this = other;
}

LocationBlock::LocationBlock(const Path &path) : _path(path) {
	this->_allowed = DEFAULT_METHODS;
	this->_dirListing = DEFAULT_DIRLISTING;
	this->_maxBodySize = DEFAULT_MAXBODYSIZE;
}

LocationBlock::~LocationBlock(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

LocationBlock &LocationBlock::operator=(const LocationBlock &other) {
	if (this == &other)
		return (*this);
	this->_path = other._path;
	this->_dirListing = other._dirListing;
	this->_maxBodySize = other._maxBodySize;
	this->_root = other._root;
	this->_allowed = other._allowed;
	this->_redirection = other._redirection;
	return (*this);
}

/* ************************************************************************** */

int32_t LocationBlock::match(const Path &target) const {
	int32_t match = this->_path.match(target);

	return match;
}

/* SETTERS ****************************************************************** */

error_t LocationBlock::allowMethod(const std::string &str) {
	const std::vector<std::string> &methods = LocationBlock::_methods;

	for (std::size_t i = 0; i < methods.size(); ++i) {
		if (0 == methods[i].compare(str)) {
			this->_allowed |= (1 << i);
			return 0;
		}
	}
	return -1;
}

error_t LocationBlock::setDirListing(const std::string &str) {
	if (0 == str.compare("on")) {
		this->_dirListing = true;
		return 0;
	}
	if (0 == str.compare("off")) {
		this->_dirListing = false;
		return 0;
	}
	return -1;
}

void LocationBlock::setMaxBodySize(const int32_t size) {
	this->_maxBodySize = size;
}

error_t LocationBlock::setRoot(const std::string &str) {
	Path path(str);

	if (!path.isOriginForm()) {
		return -1;
	}
	this->_root = path;
	return 0;
}

void LocationBlock::setRedirect(const uint16_t status, const std::string &body) {
	switch (status) {
		case MULTIPLE_CHOICES:
		case MOVED_PERMANENTLY:
		case FOUND:
		case SEE_OTHER:
		case NOT_MODIFIED:
		case TEMPORARY_REDIRECT:
		case PERMANENT_REDIRECT:
			this->_redirection.first = status;
			break;
		default:
			throw std::invalid_argument("invalid redirection status");
	}
	this->_redirection.second = body;
}

/* GETTERS ****************************************************************** */

const std::string &LocationBlock::path(void) const {
	return this->_path.string();
}

bool LocationBlock::isDirListing(void) const {
	return this->_dirListing;
}

int32_t LocationBlock::getMaxBodySize(void) const {
	return this->_maxBodySize;
}

const Path &LocationBlock::getRoot(void) const {
	return this->_root;
}

const redirect_t &LocationBlock::getRedirect(void) const {
	return this->_redirection;
}

bool LocationBlock::isAllowed(const std::string &method) const {
	const std::vector<std::string> &methods = LocationBlock::_methods;

	for (std::size_t i = 0; i < methods.size(); ++i) {
		if (0 == methods[i].compare(method)) {
			return this->_allowed & (1 << i);
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
