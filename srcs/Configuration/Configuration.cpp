#include "Configuration.hpp"

#include <iostream>

Configuration::Configuration(int argc, char *argv[])
    : _conf(&_blocks, &_mimetypes), _options(DEFAULT_OPTIONS) {
	for (int i = 1; i < argc; ++i) {
		if (_isOption(argv[i])) {
			this->_parseOption(argv[i]);
		} else if (this->_conf.path().empty()) {
			this->_conf.setPath(argv[i]);
		} else {
			throw std::invalid_argument("too many arguments");
		}
	}
	if (this->_conf.path().empty()) {
		this->_conf.setPath(DEFAULT_CONF_FILEPATH);
	}
	this->_conf.parse();
	if (0 == this->_blocks.size()) {
		throw Configuration::ConfigurationException("no \"server\" section in configuration");
	}
}

Configuration::~Configuration(void) {}

void Configuration::_parseOption(const std::string arg) {
	const std::string identifiers = "t";

	for (std::string::const_iterator c = arg.begin() + 1; c != arg.end(); ++c) {
		std::string::size_type index = identifiers.find(*c);
		if (index != std::string::npos) {
			this->_options |= (1 << index);
		} else {
			std::string message = "invalid option -- '";
			message.push_back(*c);
			message.push_back('\'');
			throw std::invalid_argument(message);
		}
	}
}

/* GETTERS ****************************************************************** */

bool Configuration::noRun(void) const {
	if (this->_options & NORUN_OPTION) {
		std::cout << "Success: the configuration file " << this->file() << " syntax is ok"
		          << std::endl;
		return true;
	}
	return false;
}

int32_t Configuration::timeout(const uint32_t type) const { return this->_conf.timeouts[type]; }

const mimetypes_t &Configuration::mimetypes(void) const { return this->_mimetypes; };

const std::string &Configuration::file(void) const { return (this->_conf.path()); }

const std::vector<ServerBlock> &Configuration::blocks(void) const { return this->_blocks; }

/* STATICS ****************************************************************** */

bool Configuration::_isOption(const char *arg) { return (*arg == '-'); }
