#include <fstream>
#include <cstring>
#include <arpa/inet.h>

#include "Configuration.hpp"
#include "ft.hpp"

void Configuration::_parseOption(const std::string arg) {
	const std::string identifiers = "t";

	for (std::string::const_iterator c = arg.begin() + 1; c != arg.end(); ++c) {
		std::string::size_type index = identifiers.find(*c);
		if (index != std::string::npos) {
			this->_options |= 1 << index;
		} else {
			std::string message = "invalid option -- '";
			message.push_back(*c);
			message.push_back('\'');
			throw std::invalid_argument(message);
		}
	}
}

/* STATICS ****************************************************************** */

bool Configuration::_isOption(const char *arg) {
	return (*arg == '-');
}
