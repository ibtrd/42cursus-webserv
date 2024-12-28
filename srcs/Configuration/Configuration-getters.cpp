#include <iostream>

#include "Configuration.hpp"

/* GETTERS ****************************************************************** */

int32_t	Configuration::backlog(void) const {
	return this->_backlog;
}

bool	Configuration::noRun(void) const {
	if (this->_options & NORUN_OPTION) {
		std::cout << "Success: the configuration file " << this->file() << " syntax is ok" << std::endl;
		return true;
	}
	return false;
}

const std::string &Configuration::file(void) const {
	return (this->_conf.path());
}
