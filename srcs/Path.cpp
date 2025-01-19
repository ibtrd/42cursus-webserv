#include "Path.hpp"

#include <errno.h>
#include <unistd.h>

#include <cstring>
#include <sstream>

/* CONSTRUCTORS ************************************************************* */

Path::Path(void) {}

Path::Path(const Path &other) { *this = other; }

Path::Path(const std::string &str) {
	std::string path = str;

	std::size_t pos = path.find("//");
	while (pos != std::string::npos) {
		path.replace(pos, 2, 1, '/');
		pos = path.find("//");
	}
	this->_str = path;

	std::stringstream ss(path);
	std::string       token;
	while (std::getline(ss, token, '/')) {
		if (!token.empty()) this->_chunks.push_back(token);
	}
}

Path::~Path(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Path &Path::operator=(const Path &other) {
	if (this == &other) return (*this);
	this->_str    = other._str;
	this->_chunks = other._chunks;
	return (*this);
}

/* ************************************************************************** */

bool Path::empty(void) const { return this->_str.empty(); }

bool Path::isOriginForm(void) const { return (!this->_str.empty() && '/' == *this->_str.begin()); }

bool Path::isFileFormat(void) const { return (!this->_str.empty() && '/' != *this->_str.rbegin()); }

bool Path::isDirFormat(void) const { return (!this->_str.empty() && '/' == *this->_str.rbegin()); }

int Path::access(int type) const { return ::access(this->_str.c_str(), type); }

error_t Path::stat(void) {
	error_t err = ::stat(this->_str.c_str(), &this->_stat);
	if (!err) this->_statDone = true;
	return err;
}

bool Path::isFile(void) const {
	if (!this->_statDone) {
		throw std::logic_error("No stat() data for this Path instance");
	}
	return S_ISREG(this->_stat.st_mode);
}

bool Path::isDir(void) const {
	if (!this->_statDone) {
		throw std::logic_error("No stat() data for this Path instance");
	}
	return S_ISDIR(this->_stat.st_mode);
}

const time_t &Path::mTime(void) const {
	if (!this->_statDone) {
		throw std::logic_error("No stat() data for this Path instance");
	}
	return this->_stat.st_mtime;
}

long Path::size(void) const {
	if (!this->_statDone) {
		throw std::logic_error("No stat() data for this Path instance");
	}
	return this->_stat.st_size;
}

std::string Path::extension(void) const {
	const std::string &src = this->_chunks.back();
	const std::size_t  pos = src.find_last_of('.');
	if (pos != std::string::npos && pos != src.size() - 1) {
		return src.substr(pos + 1);
	}
	return std::string();
}

Path Path::dir(void) const {
	const std::size_t  pos = this->_str.find_last_of('/');
	if (pos == std::string::npos) {
		return *this;
	}
	return Path(this->_str.substr(0, pos));
}

uint32_t Path::length(void) const { return this->_chunks.size(); }

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

Path Path::subPath(size_t pos, size_t count) const {
	std::string ret;

	if (count > this->_chunks.size() - pos) {
		count = this->_chunks.size() - pos;
	}
	while (count--) {
		ret += '/';
		ret += this->_chunks[pos++];
	}
	if (this->isDirFormat()) {
		ret += '/';
	}
	return ret;
}

/* GETTERS ****************************************************************** */

const std::string &Path::string(void) const { return this->_str; }

const char *Path::c_str(void) const { return this->_str.c_str(); }

/* ************************************************************************** */

std::ostream &operator<<(std::ostream &os, const Path &path) { return os << path.string(); }
