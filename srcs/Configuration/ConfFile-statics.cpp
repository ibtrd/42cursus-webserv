#include "ConfFile.hpp"

#include <cstring>

/* STATICS ****************************************************************** */

bool	ConfFile::_isMetachar(int c) {
	return (c == ';' || c == BLOCK_OPEN || c == BLOCK_CLOSE);
}

bool	ConfFile::_isComment(int c) {
	return (c == '#');
}

const struct sockaddr_in ConfFile::_defaultHost = ConfFile::_initDefaultHost();

struct sockaddr_in ConfFile::_initDefaultHost(void) {
	struct sockaddr_in host;
	std::memset(&host, 0, sizeof(host));
	host.sin_family = AF_INET;
	host.sin_addr.s_addr = INADDR_ANY;
	host.sin_port = htons(80);
	return host;
}

const ConfFile::directives ConfFile::_directives = ConfFile::_initializeDirectives();

ConfFile::directives ConfFile::_initializeDirectives(void) {
	directives map;

	map["server"] = &ConfFile::_serverDirective;
	return map;
}

const ConfFile::serverDirectives ConfFile::_serverDirectives = ConfFile::_initServerDirectives();

ConfFile::serverDirectives ConfFile::_initServerDirectives(void) {
	serverDirectives map;

	map["listen"] = &ConfFile::_listenDirective;
	map["server_name"] = &ConfFile::_serverNameDirective; 
	map["location"] = &ConfFile::_locationDirective;
	map["error_page"] = &ConfFile::_errorPageDirective;
	return map;
}

const ConfFile::locationDirectives ConfFile::_locationDirectives = ConfFile::_initLocationDirectives();

ConfFile::locationDirectives ConfFile::_initLocationDirectives(void) {
	locationDirectives map;

	map["root"] = &ConfFile::_rootDirective;
	map["allow"] = &ConfFile::_allowDirective;
	map["client_max_body_size"] = &ConfFile::_clientMaxBodySizeDirective;
	map["autoindex"] = &ConfFile::_autoindexDirective;
	map["redirect"] = &ConfFile::_redirectionDirective;
	map["index"] = &ConfFile::_indexDirective;
	// map["cgi_exe"] = &ConfFile::_cgiExeDirective;
	return map;
}
