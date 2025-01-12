#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>

#include "ConfFile.hpp"
#include "Configuration.hpp"
#include "ft.hpp"

void ConfFile::_serverDirective(std::vector<ConfToken>::const_iterator &token) {
	const uint32_t									args = this->_countBlockArgs(token);
	const std::vector<ConfToken>::const_iterator	directive = token++;

	if (0 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	
	ServerBlock		server;
	LocationBlock	globalLocation;

	globalLocation.setDefaults();
	while (++token != this->_tokens.end() && *token != BLOCK_CLOSE) {
		if (token + 1 == this->_tokens.end()) {
			throw Configuration::ConfigurationException(this->_unexpectedEOF(*token, ';', BLOCK_CLOSE));
		}
		serverDirectives::const_iterator srvdir = _serverDirectives.find(token->str());
		locationDirectives::const_iterator locdir = _locationDirectives.find(token->str());
		if (srvdir != _serverDirectives.end()) {
			(this->*(srvdir->second))(token, server);
		} else if (locdir != _locationDirectives.end()) {
			(this->*(locdir->second))(token, globalLocation);
		} else {
			throw Configuration::ConfigurationException(this->_unkwownDirective(*token));
		}
	}
	if (token == this->_tokens.end()) {
		throw Configuration::ConfigurationException(this->_unexpectedEOF(*(token - 1), BLOCK_CLOSE));
	}
	if (0 == server.hosts().size()) {
		server.addHost(_defaultHost);
	}
	server.fillLocations(globalLocation);
	this->_blocks->push_back(server);
}

void ConfFile::_listenDirective(std::vector<ConfToken>::const_iterator &token, ServerBlock &server) {
	const uint32_t 									args = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator	directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	struct sockaddr_in host = _defaultHost;
	try {
		std::size_t sep = token->str().find(':');
		if (std::string::npos != sep) {
			if (0 >= inet_pton(host.sin_family, token->str().substr(0, sep).c_str(), &host.sin_addr.s_addr)) {
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

void ConfFile::_serverNameDirective(std::vector<ConfToken>::const_iterator &token, ServerBlock &server) {
	const uint32_t 									args = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator	directive = token++;

	if (0 == args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	for (uint32_t i = 0; i < args; ++i) {
		server.addName(token->str());
		++token;
	}
}
