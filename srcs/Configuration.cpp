#include <stdexcept>
#include <fstream>
#include <iostream>

#include "Configuration.hpp"

/* CONSTRUCTORS ************************************************************* */

Configuration::Configuration(void) : _backlog(DEFAULT_BACKLOG) {}

Configuration::Configuration(const Configuration &other)
{
	*this = other;
}

Configuration::Configuration(int argc, char *argv[])
{
	if (argc > 2)
	{
		throw std::invalid_argument("too many arguments");
	}
	this->_parseFile(argc == 2 ? argv[1] : DEFAULT_CONF_FILEPATH);
}

Configuration::~Configuration(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Configuration &Configuration::operator=(const Configuration &other)
{
	if (this == &other)
		return (*this);
	this->_backlog = other._backlog;
	this->_blocks = other._blocks;
	return (*this);
}

/* ************************************************************************** */

void Configuration::_parseFile(const char *filepath)
{

	std::ifstream conf(filepath, std::ios::in);
	if (false == conf.is_open())
	{
		throw std::invalid_argument("couldn't open configuration file");
	}

	while (false == conf.eof())
	{
		std::string directive;

		DirectiveMap::const_iterator it = directives.find(directive);
		if (it != directives.end()) {
			(this->*(it->second))();
		} else {
			std::cerr << "Unknown directive: " << directive << std::endl;
		}
	}
}

void Configuration::_serverDirective(void)
{
	std::cout << "test!" << std::endl;
}

/* GETTERS ****************************************************************** */

int32_t Configuration::backlog(void) const { return this->_backlog; }

/* SETTERS ****************************************************************** */

void	Configuration::setBacklog(const int32_t backlog) { this->_backlog = backlog; }

/* STATICS ****************************************************************** */

const Configuration::DirectiveMap Configuration::directives = Configuration::_initializeDirectives();

Configuration::DirectiveMap Configuration::_initializeDirectives(void) {
	DirectiveMap map;

	map["server"] = &Configuration::_serverDirective;
	return map;
}
