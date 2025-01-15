#include "Path.hpp"

#include <sstream>
#include <unistd.h>
#include <errno.h>
#include <cstring>

/* CONSTRUCTORS ************************************************************* */

Path::Path(void) {}

Path::Path(const Path &other) {
	*this = other;
}

Path::Path(const std::string &str) {
	std::string	path = str;

	std::size_t pos = path.find("//");
	while (pos != std::string::npos) {
		path.replace(pos, 2, 1, '/');
		pos = path.find("//");
	}
	this->_str = path;

	std::stringstream	ss(path);
	std::string			token;
	while (std::getline(ss, token, '/')) {
		if (!token.empty())
			this->_chunks.push_back(token);
	}
}

Path::~Path(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Path &Path::operator=(const Path &other) {
	if (this == &other)
		return (*this);
	this->_str = other._str;
	this->_chunks = other._chunks;
	return (*this);
}

/* ************************************************************************** */

bool Path::isOriginForm(void) const {
	return (!this->_str.empty() && '/' == *this->_str.begin());
}

bool Path::isFileFormat(void) const {
	return (!this->_str.empty() && '/' != *this->_str.rbegin());
}

bool Path::isDirFormat(void) const {
	return (!this->_str.empty() && '/' == *this->_str.rbegin());
}

bool Path::exists(void) {
	if (access(this->_str.c_str(), F_OK) == -1) {
		return false;
	}
	if (stat(this->_str.c_str(), &this->_stat) == -1) {
		throw std::runtime_error("stat: " + std::string(std::strerror(errno)));
	}
	return (true);
}

bool Path::hasPermission(int32_t mode) const {
	return (access(this->_str.c_str(), mode) != -1);
}

bool Path::isFile(void) const {
	return S_ISREG(this->_stat.st_mode);
}

bool Path::isDir(void) const {
	return S_ISDIR(this->_stat.st_mode);
}

std::string Path::extension(void) const {
	const std::string &src = this->_chunks.back();
	const std::size_t pos = src.find_last_of('.');
	if (pos != std::string::npos && pos != src.size() - 1) {
		return src.substr(pos + 1);
	}
	return std::string();
}

uint32_t Path::length(void) const {
	return this->_chunks.size();
}

uint32_t Path::prefixLength(void) const {
	if (this->isDirFormat()) {
		return this->_chunks.size();
	}
	return this->_chunks.size() == 0 ? 0 : this->_chunks.size() - 1;
}

bool Path::prefixMatch(const Path &other) const {
	if (this->_chunks.size() > other._chunks.size()) {
		return false;
	}
	for (uint32_t i = 0; i < this->_chunks.size(); ++i) {
		if (this->_chunks[i] != other._chunks[i]) {
			return false;
		}
	}
	return true;
}

std::string Path::concat(const Path &other) const {
	if (this->isDirFormat() && other.isOriginForm()) {
		return this->_str + other._str.substr(1);
	}
	return this->_str + other._str;
}

/* GETTERS ****************************************************************** */

const std::string &Path::string(void) const {
	return this->_str;
}

/* ************************************************************************** */

std::ostream &operator<<(std::ostream &os, const Path &path) {
	return os << path.string();
}
