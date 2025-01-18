#include <cstring>
#include <fstream>
#include <sstream>

#include "Configuration.hpp"
#include "ft.hpp"

#define DEFAULT_CLIENT_HEADER_TIMEOUT 10
#define DEFAULT_CLIENT_BODY_TIMEOUT 60
#define DEFAULT_SEND_TIMEOUT 60

/* CONSTRUCTORS ************************************************************* */

ConfFile::ConfFile(std::vector<ServerBlock> *blocks) : _blocks(blocks) {
	this->timeouts[CLIENT_HEADER_TIMEOUT] = DEFAULT_CLIENT_HEADER_TIMEOUT;
	this->timeouts[CLIENT_BODY_TIMEOUT]   = DEFAULT_CLIENT_BODY_TIMEOUT;
	this->timeouts[SEND_TIMEOUT]          = DEFAULT_SEND_TIMEOUT;
}

ConfFile::~ConfFile(void) {}

/* ************************************************************************** */

void ConfFile::parse(void) {
	std::ifstream conf(this->_path.c_str(), std::ios::in);

	if (false == conf.is_open()) {
		throw Configuration::ConfigurationException(this->_path + ": " + std::strerror(errno));
	}

	std::string line;
	uint32_t    index = 0;

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

uint32_t ConfFile::_countArgs(const std::vector<ConfToken>::const_iterator &directive) const {
	std::vector<ConfToken>::const_iterator token = directive;
	uint32_t                               args  = 0;

	while (++token != this->_tokens.end()) {
		if (*token == ';') {
			return args;
		} else if (token->isMetatoken()) {
			throw Configuration::ConfigurationException(this->_unexpectedToken(*token));
		}
		args++;
	}
	throw Configuration::ConfigurationException(this->_unexpectedEOF(*directive, ';'));
}

uint32_t ConfFile::_countBlockArgs(const std::vector<ConfToken>::const_iterator &directive) const {
	std::vector<ConfToken>::const_iterator token = directive;
	uint32_t                               args  = 0;

	while (++token != this->_tokens.end()) {
		if (*token == BLOCK_OPEN) {
			return args;
		} else if (token->isMetatoken()) {
			throw Configuration::ConfigurationException(this->_unexpectedToken(*token));
		}
		args++;
	}
	throw Configuration::ConfigurationException(this->_missingOpening(*directive, BLOCK_OPEN));
}

/* GETTERS ****************************************************************** */

const std::string &ConfFile::path(void) const { return this->_path; }

const std::vector<ConfToken> &ConfFile::tokens(void) const { return this->_tokens; }

bool ConfFile::eof(std::vector<ConfToken>::const_iterator &it) const {
	return (this->_tokens.end() == it);
}

/* SETTERS ****************************************************************** */

void ConfFile::setPath(const std::string &path) { this->_path = path; }
