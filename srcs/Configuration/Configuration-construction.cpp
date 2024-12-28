#include <cstring>
#include <stdexcept>

#include "Configuration.hpp"

Configuration::Configuration(int argc, char *argv[]) :
	_conf(&_blocks),
	_options(DEFAULT_OPTIONS),
	_backlog(DEFAULT_BACKLOG)
{
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
}
	// std::ifstream confFile(this->_filePath.c_str(), std::ios::in);

	// this->_tokenizeFile(confFile);
	// this->_parseTokens();

Configuration::~Configuration(void) {}
