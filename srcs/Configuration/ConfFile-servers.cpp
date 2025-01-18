#include <arpa/inet.h>

#include "Configuration.hpp"
#include "ft.hpp"

void ConfFile::_serverDirective(std::vector<ConfToken>::const_iterator &token) {
	const uint32_t                               args      = this->_countBlockArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (0 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}

	ServerBlock   server;
	LocationBlock globalLocation;

	this->_root = Path();
	globalLocation.setDefaults();
	while (++token != this->_tokens.end() && *token != BLOCK_CLOSE) {
		if (token + 1 == this->_tokens.end()) {
			throw Configuration::ConfigurationException(
			    this->_unexpectedEOF(*token, ';', BLOCK_CLOSE));
		}
		serverDirectives::const_iterator   srvdir = _serverDirectives.find(token->str());
		locationDirectives::const_iterator locdir = _locationDirectives.find(token->str());
		if (srvdir != _serverDirectives.end()) {
			(this->*(srvdir->second))(token, server);
		} else if (locdir != _locationDirectives.end()) {
			(this->*(locdir->second))(token, globalLocation);
			this->_root = globalLocation.getRoot();
		} else {
			throw Configuration::ConfigurationException(this->_unkwownDirective(*token));
		}
	}
	if (token == this->_tokens.end()) {
		throw Configuration::ConfigurationException(
		    this->_unexpectedEOF(*(token - 1), BLOCK_CLOSE));
	}
	if (0 == server.hosts().size()) {
		server.addHost(_defaultHost);
	}
	server.fillLocations(globalLocation);
	this->_blocks->push_back(server);
}

void ConfFile::_listenDirective(std::vector<ConfToken>::const_iterator &token,
                                ServerBlock                            &server) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	struct sockaddr_in host = _defaultHost;
	try {
		std::size_t sep = token->str().find(':');
		if (std::string::npos != sep) {
			if (0 >= inet_pton(host.sin_family, token->str().substr(0, sep).c_str(),
			                   &host.sin_addr.s_addr)) {
				throw std::invalid_argument("");
			}
			host.sin_port = htons(ft::stoi<in_addr_t>(token->str().substr(sep + 1)));
		} else {
			if (0 >= inet_pton(host.sin_family, token->str().c_str(), &host.sin_addr.s_addr)) {
				host.sin_port = htons(ft::stoi<in_addr_t>(token->str()));
			}
		}
	} catch (std::invalid_argument &e) {
		throw Configuration::ConfigurationException(this->_hostNotFound(*directive, *token));
	}
	server.addHost(host);
	++token;
}

void ConfFile::_serverNameDirective(std::vector<ConfToken>::const_iterator &token,
                                    ServerBlock                            &server) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (0 == args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	for (uint32_t i = 0; i < args; ++i) {
		server.addName(token->str());
		++token;
	}
}

void ConfFile::_errorPageDirective(std::vector<ConfToken>::const_iterator &token,
                                   ServerBlock                            &server) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (2 > args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	if (this->_root.empty()) {
		token += args;
		return;
	}
	Path file(this->_root.string() + (token + args - 1)->str());
	for (uint32_t i = 0; i < args - 1; ++i) {
		try {
			status_code_t code = ft::stoi<status_code_t>(token->str());
			if (code < 400 || code > 599) {
				throw std::invalid_argument("");
			}
			server.addErrorPage(code, file);
		} catch (std::invalid_argument &e) {
			throw Configuration::ConfigurationException(this->_invalidValue(*directive, *token));
		}
		++token;
	}
	++token;
}
