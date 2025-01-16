#include "Method.hpp"

#include <algorithm>
#include <stdexcept>

/* CONSTRUCTORS ************************************************************* */

Method::Method(void) : _index(INVAL_METHOD) {}

Method::Method(const Method &other) { *this = other; }

Method::Method(const std::string &str) {
	std::vector<std::string>::const_iterator it = std::find(methods.begin(), methods.end(), str);
	if (it == methods.end()) {
		this->_index = INVAL_METHOD;
	} else {
		this->_index = (method_t)std::distance(methods.begin(), it);
	}
}

Method::~Method(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Method &Method::operator=(const Method &other) {
	if (this == &other) return (*this);
	this->_index = other._index;
	return (*this);
}

bool Method::operator==(const Method &other) { return (this->_index == other._index); }

bool Method::operator==(const method_t method) { return (this->_index == method); }

bool Method::operator!=(const Method &other) { return (this->_index != other._index); }

bool Method::operator!=(const method_t method) { return (this->_index != method); }

/* ************************************************************************** */

bool Method::isValid(void) const { return (this->_index != INVAL_METHOD); }

/* GETTERS ****************************************************************** */

method_t Method::index(void) const { return this->_index; }

const std::string &Method::string(void) const {
	if (this->isValid()) return methods[this->_index];
	throw std::invalid_argument("Invalid method");
}

/* EXCEPTIONS *************************************************************** */

/* STATICS ****************************************************************** */

const std::vector<std::string> Method::methods = Method::_initMethods();

std::vector<std::string> Method::_initMethods(void) {
	std::vector<std::string> methods;
	methods.push_back("GET");
	methods.push_back("POST");
	methods.push_back("DELETE");
	methods.push_back("PUT");
	return methods;
}
