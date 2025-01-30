#include <cstring>

#include "ConfFile.hpp"

bool ConfFile::_isMetachar(int c) { return (c == ';' || c == BLOCK_OPEN || c == BLOCK_CLOSE); }

bool ConfFile::_isComment(int c) { return (c == '#'); }

const struct sockaddr_in ConfFile::_defaultHost = ConfFile::_initDefaultHost();

struct sockaddr_in ConfFile::_initDefaultHost(void) {
	struct sockaddr_in host;
	std::memset(&host, 0, sizeof(host));
	host.sin_family      = AF_INET;
	host.sin_addr.s_addr = INADDR_ANY;
	host.sin_port        = htons(80);
	return host;
}

const ConfFile::directives ConfFile::_directives = ConfFile::_initializeDirectives();

ConfFile::directives ConfFile::_initializeDirectives(void) {
	directives map;

	map["server"]                = &ConfFile::_serverDirective;
	map["client_header_timeout"] = &ConfFile::_clientHeaderTimeoutDirective;
	map["client_body_timeout"]   = &ConfFile::_clientBodyTimeoutDirective;
	map["send_timeout"]          = &ConfFile::_sendTimeoutDirective;
	map["types"]                 = &ConfFile::_typesDirective;
	return map;
}

const ConfFile::serverDirectives ConfFile::_serverDirectives = ConfFile::_initServerDirectives();

ConfFile::serverDirectives ConfFile::_initServerDirectives(void) {
	serverDirectives map;

	map["listen"]      = &ConfFile::_listenDirective;
	map["server_name"] = &ConfFile::_serverNameDirective;
	map["location"]    = &ConfFile::_locationDirective;
	map["error_page"]  = &ConfFile::_errorPageDirective;
	return map;
}

const ConfFile::locationDirectives ConfFile::_locationDirectives =
    ConfFile::_initLocationDirectives();

ConfFile::locationDirectives ConfFile::_initLocationDirectives(void) {
	locationDirectives map;

	map["root"]                  = &ConfFile::_rootDirective;
	map["allow"]                 = &ConfFile::_allowDirective;
	map["client_max_body_size"]  = &ConfFile::_clientMaxBodySizeDirective;
	map["autoindex"]             = &ConfFile::_autoindexDirective;
	map["redirect"]              = &ConfFile::_redirectionDirective;
	map["index"]                 = &ConfFile::_indexDirective;
	map["cgi"]                   = &ConfFile::_cgiDirective;
	map["client_body_upload_path"] = &ConfFile::_clientBodyUploadPathDirective;
	map["client_body_temp_path"] = &ConfFile::_clientBodyTempPathDirective;
	return map;
}
