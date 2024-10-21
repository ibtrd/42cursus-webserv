#include "Configuration.hpp"

/* CONSTRUCTORS ************************************************************* */

Configuration::Configuration(void) : _port(-1), _backlog(511) {}

Configuration::Configuration(const Configuration &other) {
	*this = other;
}

Configuration::~Configuration(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Configuration	&Configuration::operator=(const Configuration &other) {
	if (this == &other)
		return (*this);
	this->_port = other._port;
	this->_backlog = other._backlog;
	this->_name = other._name;
	return (*this);
}

/* ************************************************************************** */

/* GETTERS ****************************************************************** */

int32_t Configuration::port(void) const { return this->_port; }
int32_t Configuration::backlog(void) const { return this->_backlog; }
const std::string &Configuration::name(void) const { return this->_name; }

/* SETTERS ****************************************************************** */

void Configuration::setPort(const int32_t port) { this->_port = port; }
void Configuration::setBacklog(const int32_t backlog) { this->_backlog = backlog; }
void Configuration::setName(const std::string &name) { this->_name = name; }

/* EXCEPTIONS *************************************************************** */
