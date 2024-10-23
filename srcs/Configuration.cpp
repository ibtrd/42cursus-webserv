#include <stdexcept>

#include "Configuration.hpp"

# define CONFIG_DEFAULT_PATH "./config"

/* CONSTRUCTORS ************************************************************* */

Configuration::Configuration(void) : _port(-1), _backlog(511) {}

Configuration::Configuration(const Configuration &other) {
	*this = other;
}

Configuration::Configuration(int argc, char *argv[]) {
	if (argc > 2) {
		throw std::invalid_argument("too many arguments");
	}
	const std::string	filepath(argc == 2 ? argv[1] : CONFIG_DEFAULT_PATH);

	this->_port = 8080;
	this->_backlog = 511;
}

Configuration::~Configuration(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Configuration	&Configuration::operator=(const Configuration &other) {
	if (this == &other)
		return (*this);
	this->_port = other._port;
	this->_backlog = other._backlog;
	return (*this);
}

/* ************************************************************************** */

/* GETTERS ****************************************************************** */

int32_t Configuration::port(void) const { return this->_port; }
int32_t Configuration::backlog(void) const { return this->_backlog; }

/* SETTERS ****************************************************************** */

void Configuration::setPort(const int32_t port) { this->_port = port; }
void Configuration::setBacklog(const int32_t backlog) { this->_backlog = backlog; }

