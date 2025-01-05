#include "ConfFile.hpp"

/* STATICS ****************************************************************** */

bool	ConfFile::_isMetachar(int c) {
	return (c == ';' || c == BLOCK_OPEN || c == BLOCK_CLOSE);
}

bool	ConfFile::_isComment(int c) {
	return (c == '#');
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
	// map["cgi_exe"] = &ConfFile::_cgiExeDirective;
	return map;
}
