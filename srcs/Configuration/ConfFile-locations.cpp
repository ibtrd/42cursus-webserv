#include "ConfFile.hpp"
#include "Configuration.hpp"
#include "ft.hpp"

void ConfFile::_locationDirective(std::vector<ConfToken>::const_iterator &token, ServerBlock &server) {
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (token->isMetatoken()) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	} else if (++token == this->_tokens.end() || *token != BLOCK_OPEN) {
		throw Configuration::ConfigurationException(this->_missingOpening(*directive, BLOCK_OPEN));
	}

	LocationBlock	location(token->str());

	while (++token != this->_tokens.end() && !token->isMetatoken()) {
		locationDirectives::const_iterator dir = _locationDirectives.find(token->str());
		if (dir != _locationDirectives.end() && token + 1 != this->_tokens.end()) {
			(this->*(dir->second))(token, location);
		} else if (dir == _locationDirectives.end()) {
			throw Configuration::ConfigurationException(this->_unkwownDirective(*token));
		} else {
			throw Configuration::ConfigurationException(this->_unexpectedEOF(*token, ';', BLOCK_CLOSE));
		}
	}
	if (token == this->_tokens.end()) {
		throw Configuration::ConfigurationException(this->_unexpectedEOF(*(token - 1), BLOCK_CLOSE));
	} else if (*token != BLOCK_CLOSE) {
		throw Configuration::ConfigurationException(this->_unexpectedToken(*token));
	}
	server.addLocation(location);
}

void ConfFile::_rootDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location) {
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (*token == ';') {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	} else if (token->isMetatoken()) {
		throw Configuration::ConfigurationException(this->_unexpectedToken(*token));
	}
	location.setRoot(token->str());
	++token;
	if (token == this->_tokens.end()) {
		throw Configuration::ConfigurationException(this->_unexpectedEOF(*token, ';'));
	} else if (*token == ';') {
		return;
	} else if (token->isMetatoken()) {
		throw Configuration::ConfigurationException(this->_unexpectedToken(*token));
	} else {

	}
}

void ConfFile::_allowDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location) {
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (*token == ';') {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	} else if (token->isMetatoken()) {
		throw Configuration::ConfigurationException(this->_unexpectedToken(*token));
	}

	while (token != this->_tokens.end() && !token->isMetatoken()) {
		if (false == location.allowMethod(token->str())) {
			throw Configuration::ConfigurationException(this->_invalidMethod(*token));
		}
		++token;
	}

	if (token == this->_tokens.end()) {
		throw Configuration::ConfigurationException(this->_unexpectedEOF(*token, ';'));
	} else if (*token != ';') {
		throw Configuration::ConfigurationException(this->_unexpectedToken(*token));
	}
}

void ConfFile::_clientMaxBodySizeDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location) {
	const uint32_t									n = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator	directive = token++;
	
	if (1 != n) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	try {
		location.setMaxBodySize(ft::stoi<uint32_t>(token->str()));
		++token;
	} catch (std::invalid_argument &e) {
		throw Configuration::ConfigurationException(this->_invalidValue(*directive, *token));
	}
}

void ConfFile::_autoindexDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location) {
	const uint32_t									n = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator	directive = token++;

	if (1 != n) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	if (false == location.setDirListing(token->str())) {
		throw Configuration::ConfigurationException(this->_invalidValue(*directive, *token, "on", "off"));
	}
	++token;
}

