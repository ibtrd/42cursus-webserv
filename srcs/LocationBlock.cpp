
#include "LocationBlock.hpp"
#include "Method.hpp"

/* CONSTRUCTORS ************************************************************* */

LocationBlock::LocationBlock(void) {
	this->_allowed = DEFAULT_ALLOW;
	this->_dirListing = DEFAULT_DIRLISTING;
	this->_maxBodySize = DEFAULT_MAXBODYSIZE;
	this->_redirection = DEFAULT_REDIRECTON;
}

LocationBlock::LocationBlock(const LocationBlock &other) {
	*this = other;
}

LocationBlock::LocationBlock(const Path &path) : _path(path) {
	this->_allowed = DEFAULT_ALLOW;
	this->_dirListing = DEFAULT_DIRLISTING;
	this->_maxBodySize = DEFAULT_MAXBODYSIZE;
	this->_redirection = DEFAULT_REDIRECTON;
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

bool LocationBlock::match(const Path &target) const {
	return this->_path.prefixMatch(target);
}

void LocationBlock::fill(const LocationBlock &other) {
	if (-1 == this->_dirListing) {
		this->_dirListing = other._dirListing;
	}
	if (-1 == this->_maxBodySize) {
		this->_maxBodySize = other._maxBodySize;
	}
	if (this->_root.string().empty()) {
		this->_root = other._root;
	}
	if (!this->_allowed) {
		this->_allowed = other._allowed;
	}
	if (0 == this->_redirection.first) {
		this->_redirection = other._redirection;
	}
}

/* SETTERS ****************************************************************** */

error_t LocationBlock::allowMethod(const std::string &str) {
	for (std::size_t i = 0; i < Method::methods.size(); ++i) {
		if (0 == Method::methods[i].compare(str)) {
			this->_allowed |= (1 << i);
			return 0;
		}
	}
	return -1;
}

error_t LocationBlock::setDirListing(const std::string &str) {
	if (0 == str.compare(DIRLISTING_ON)) {
		this->_dirListing = 1;
		return 0;
	}
	if (0 == str.compare(DIRLISTING_OFF)) {
		this->_dirListing = 0;
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

const Path &LocationBlock::path(void) const {
	return this->_path;
}

bool LocationBlock::isDirListing(void) const {
	return this->_dirListing == 1;
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

bool LocationBlock::isAllowed(const Method &method) const {
	return this->_allowed & (1 << method.index());
}

std::ostream &operator<<(std::ostream &os, const LocationBlock &location) {
	os << "Location '" << location._path << "' {\n"
		<< "\troot: " << location._root << "\n"
		<< "\tdirListing: ";
	if (location._dirListing == -1) {
		os << "undefined";
	} else {
		os << (location._dirListing ? DIRLISTING_ON : DIRLISTING_OFF);
	}
		os << "\n\tallowed: ";
	for (uint32_t i = 0; i < Method::methods.size(); ++i) {
		if (location.isAllowed(Method::methods[i])) {
			os << Method::methods[i] << " ";
		}
	}
	os << "\n\tmaxBodySize: " << location._maxBodySize << "\n}" << std::endl;
	return os;
}
