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
	const uint32_t									args = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator	directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	location.setRoot(token->str());
	++token;
}

void ConfFile::_allowDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location) {
	const uint32_t									args = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator	directive = token++;

	if (0 == args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	for (uint32_t i = 0; i < args; ++i) {
		if (0 != location.allowMethod(token->str())) {
			throw Configuration::ConfigurationException(this->_invalidMethod(*token));
		}
		++token;
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
	if (0 != location.setDirListing(token->str())) {
		throw Configuration::ConfigurationException(this->_invalidValue(*directive, *token, "on", "off"));
	}
	++token;
}

void ConfFile::_redirectionDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location) {
	const uint32_t									args = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator	directive = token++;

	if (2 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	try {
		uint16_t status = ft::stoi<uint16_t>(token->str());
		++token;
		location.setRedirect(status, token->str());
		++token;
	} catch (std::invalid_argument &e) {
		throw Configuration::ConfigurationException(this->_invalidValue(*directive, *token));
	}
}
