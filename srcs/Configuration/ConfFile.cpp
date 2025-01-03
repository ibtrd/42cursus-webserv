#include <fstream>
#include <cstring>
#include <sstream>
#include <arpa/inet.h>

#include "Configuration.hpp"
#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

ConfFile::ConfFile(std::vector<ServerBlock> *blocks) : _blocks(blocks) {}

ConfFile::~ConfFile(void) {}

/* ************************************************************************** */

# include <iostream>

void ConfFile::parse(void) {
	std::ifstream	conf(this->_path.c_str(), std::ios::in);

	if (false == conf.is_open()) {
		throw Configuration::ConfigurationException(this->_path + ": " + std::strerror(errno));
	}

	std::string line;
	uint32_t	index = 0;

	while (std::getline(conf, line)) {
		this->_tokenize(line, ++index);
	}

	std::vector<ConfToken>::const_iterator token = this->_tokens.begin();

	while (token != this->_tokens.end()) {
		directives::const_iterator dir = _directives.find(token->str());
		if (dir != _directives.end()) {
			(this->*(dir->second))(token);
		} else {
			throw Configuration::ConfigurationException(this->_unkwownDirective(*token));
		}
		++token;
	}
}

/* ************************************************************************** */

void ConfFile::_tokenize(const std::string &line, const uint32_t index) {
	std::string token;

	for (std::string::const_iterator it = line.begin(); it != line.end(); ++it) {
		if (_isComment(*it)) {
			break;
		}
		if (isspace(*it) || _isMetachar(*it)) {
			if (!token.empty()) {
				this->_tokens.push_back(ConfToken(token, index));
				token.clear();
			}
			if (_isMetachar(*it)) {
				this->_tokens.push_back(ConfToken(std::string(1, *it), index));
			}
		} else {
			token.push_back(*it);
		}
	}
	if (!token.empty()) {
		this->_tokens.push_back(ConfToken(token, index));
	}
}

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

uint32_t ConfFile::_countArgs(const std::vector<ConfToken>::const_iterator &directive) const {
	std::vector<ConfToken>::const_iterator	token = directive;
	uint32_t 								args = 0;

	while (++token != this->_tokens.end()) {
		if (*token == ';' ) {
			return args;
		} else if (token->isMetatoken()) {
			throw Configuration::ConfigurationException(this->_unexpectedToken(*token));
		}
		args++;
	}
	throw Configuration::ConfigurationException(this->_unexpectedEOF(*directive, ';'));
}

/* GETTERS ****************************************************************** */

const std::string &ConfFile::path(void) const {
	return this->_path;
}

const std::vector<ConfToken> &ConfFile::tokens(void) const {
	return this->_tokens;
}

bool ConfFile::eof(std::vector<ConfToken>::const_iterator &it) const {
	return (this->_tokens.end() == it);
}

/* SETTERS ****************************************************************** */

void ConfFile::setPath(const std::string &path) {
	this->_path = path;
}

/* EXCEPTIONS *************************************************************** */
