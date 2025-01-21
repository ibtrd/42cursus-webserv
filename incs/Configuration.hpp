#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "ConfFile.hpp"

#define DEFAULT_CONF_FILEPATH "./conf/default.conf"
#define DEFAULT_BACKLOG 511
#define DEFAULT_OPTIONS 0

#define NORUN_OPTION (1 << 0)

class Configuration {
public:
	Configuration(int argc, char *argv[]);
	~Configuration(void);

	// GETTERS
	bool               noRun(void) const;
	int32_t            timeout(const uint32_t type) const;
	const mimetypes_t &mimetypes(void) const;

	const std::string              &file(void) const;
	const std::vector<ServerBlock> &blocks(void) const;

	// EXCEPTION
	class ConfigurationException : public std::exception {
	public:
		ConfigurationException(const std::string &message) : _message(message) {}
		virtual ~ConfigurationException() throw() {}

		const char *what() const throw() { return _message.c_str(); }

	private:
		std::string _message;
	};

private:
	ConfFile                 _conf;
	uint8_t                  _options;
	std::vector<ServerBlock> _blocks;
	mimetypes_t              _mimetypes;

	void _parseOption(const std::string arg);

	static bool _isOption(const char *str);
};

#endif /* ******************************************************************* */
