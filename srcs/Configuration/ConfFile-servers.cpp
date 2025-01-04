#include <arpa/inet.h>
#include <stdexcept>

#include "ConfFile.hpp"
#include "Configuration.hpp"
#include "ft.hpp"

void ConfFile::_serverDirective(std::vector<ConfToken>::const_iterator &token) {
	std::vector<ConfToken>::const_iterator directive = token++;

	if (token == this->_tokens.end() || *token != BLOCK_OPEN) {
		throw Configuration::ConfigurationException(this->_missingOpening(*directive, BLOCK_OPEN));	
	}
	
	ServerBlock	server;

	while (++token != this->_tokens.end() && *token != BLOCK_CLOSE) {
		serverDirectives::const_iterator dir = _serverDirectives.find(token->str());
		if (dir != _serverDirectives.end() && token + 1 != this->_tokens.end()) {
			(this->*(dir->second))(token, server);
		} else if (dir == _serverDirectives.end()) {
			throw Configuration::ConfigurationException(this->_unkwownDirective(*token));
		} else {
			throw Configuration::ConfigurationException(this->_unexpectedEOF(*token, ';', '}'));
		}
	}
	if (token == this->_tokens.end()) {
		throw Configuration::ConfigurationException(this->_unexpectedEOF(*(token - 1), BLOCK_CLOSE));
	}
	this->_blocks->push_back(server);
}

void ConfFile::_listenDirective(std::vector<ConfToken>::const_iterator &token, ServerBlock &server) {
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (token == this->_tokens.end() || *token == ';' || token + 1 == this->_tokens.end() || *(token + 1) != ';') {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	
	std::pair<in_addr_t, in_port_t> host;

	try {
		std::size_t sep = token->str().find(':');
		if (std::string::npos != sep) {
			if (0 >= inet_pton(AF_INET, token->str().substr(0, sep).c_str(), &host.first)) {
				throw std::invalid_argument("");
			}
			host.second = ft::stoi<in_addr_t>(token->str().substr(sep + 1));
		} else {
			host.first = INADDR_ANY;
			host.second = ft::stoi<in_addr_t>(token->str());
		}
	} catch (std::invalid_argument &e) {
		throw Configuration::ConfigurationException(this->_hostNotFound(*directive, *token));
	}
	server.addHost(host);
	++token;
}

void ConfFile::_serverNameDirective(std::vector<ConfToken>::const_iterator &token, ServerBlock &server) {
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (*token == ';') {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	while (token != this->_tokens.end() && !token->isMetatoken()) {
		server.addName(token->str());
		++token;
	}
	if (token == this->_tokens.end()) {
		throw Configuration::ConfigurationException(this->_unexpectedEOF(*directive, ';'));
	} else if (*token != ';') {
		throw Configuration::ConfigurationException(this->_unexpectedToken(*token));
	}
}
