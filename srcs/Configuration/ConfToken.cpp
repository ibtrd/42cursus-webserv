#include "ConfToken.hpp"

/* CONSTRUCTORS ************************************************************* */

ConfToken::ConfToken(void) {}

ConfToken::ConfToken(const ConfToken &other) {
	*this = other;
}

ConfToken::ConfToken(const std::string &str, const uint32_t line) :
	_str(str),
	_line(line) {}

ConfToken::~ConfToken(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

ConfToken &ConfToken::operator=(const ConfToken &other) {
	if (this == &other)
		return (*this);
	this->_str = other._str;
	this->_line = other._line;
	return (*this);
}

bool ConfToken::operator==(const std::string &str) const {
	return (0 == this->_str.compare(str));
}

bool ConfToken::operator==(const char c) const {
	return (0 == this->_str.compare(std::string(1, c)));
}

bool ConfToken::operator!=(const std::string &str) const {
	return (0 != this->_str.compare(str));
}

bool ConfToken::operator!=(const char c) const {
	return (0 != this->_str.compare(std::string(1, c)));
}

/* ************************************************************************** */

bool ConfToken::isMetatoken(void) const {
	return (*this == BLOCK_OPEN || *this == BLOCK_CLOSE || *this == ';');
}

/* GETTERS ****************************************************************** */

const std::string &ConfToken::str(void) const {
	return this->_str;
}

uint32_t ConfToken::line(void) const {
	return this->_line;
}
