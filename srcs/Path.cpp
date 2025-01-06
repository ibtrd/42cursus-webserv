#include "Path.hpp"

#include <sstream>

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

#include <iostream>

uint32_t Path::length(void) const {
	return this->_chunks.size();
}

int32_t Path::match(const Path &other) const {
	uint32_t i = 0;
	while (i < this->_chunks.size() && i < other._chunks.size() && this->_chunks[i] == other._chunks[i]) {
		i++;
	}
	// if (this->isDir() && other.isDir())
	// 	i--;
	
	return i;
}

bool Path::isOriginForm(void) const {
	return (!this->_str.empty() && '/' == *this->_str.begin());
}

bool Path::isFile(void) const {
	return (!this->_str.empty() && '/' != *this->_str.rbegin());
}

bool Path::isDir(void) const {
	return (!this->_str.empty() && '/' == *this->_str.rbegin());
}

std::string Path::extension(void) const {
	const std::string &src = this->_chunks.back();
	const std::size_t pos = src.find('.');
	if (pos != std::string::npos) {
		return src.substr(pos);
	}
	return std::string();
}

/* GETTERS ****************************************************************** */

const std::string &Path::string(void) const {
	return this->_str;
}
