#include <iostream>

#include "ConfFile.hpp"
#include "Configuration.hpp"
#include "ft.hpp"

void ConfFile::_locationDirective(std::vector<ConfToken>::const_iterator &token,
                                  ServerBlock                            &server) {
	const uint32_t                               args      = this->_countBlockArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}

	Path path(token->str() + '/');
	if (!path.isOriginForm()) {
		throw Configuration::ConfigurationException(this->_invalidPath(*directive, *token));
	}
	++token;

	LocationBlock location(path);

	while (++token != this->_tokens.end() && !token->isMetatoken()) {
		locationDirectives::const_iterator dir = _locationDirectives.find(token->str());
		if (dir != _locationDirectives.end() && token + 1 != this->_tokens.end()) {
			(this->*(dir->second))(token, location);
		} else if (dir == _locationDirectives.end()) {
			throw Configuration::ConfigurationException(this->_unkwownDirective(*token));
		} else {
			throw Configuration::ConfigurationException(
			    this->_unexpectedEOF(*token, ';', BLOCK_CLOSE));
		}
	}
	if (token == this->_tokens.end()) {
		throw Configuration::ConfigurationException(
		    this->_unexpectedEOF(*(token - 1), BLOCK_CLOSE));
	} else if (*token != BLOCK_CLOSE) {
		throw Configuration::ConfigurationException(this->_unexpectedToken(*token));
	}
	server.addLocation(location);
}

void ConfFile::_rootDirective(std::vector<ConfToken>::const_iterator &token,
                              LocationBlock                          &location) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	if (0 != location.setRoot(token->str() + '/')) {
		throw Configuration::ConfigurationException(this->_invalidPath(*directive, *token));
	}
	++token;
}

void ConfFile::_allowDirective(std::vector<ConfToken>::const_iterator &token,
                               LocationBlock                          &location) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

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

void ConfFile::_clientMaxBodySizeDirective(std::vector<ConfToken>::const_iterator &token,
                                           LocationBlock                          &location) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	try {
		location.setMaxBodySize(ft::stoi<uint32_t>(token->str()));
		++token;
	} catch (std::exception &e) {
		throw Configuration::ConfigurationException(this->_invalidValue(*directive, *token));
	}
}

void ConfFile::_autoindexDirective(std::vector<ConfToken>::const_iterator &token,
                                   LocationBlock                          &location) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	if (0 != location.setDirListing(token->str())) {
		throw Configuration::ConfigurationException(
		    this->_invalidValue(*directive, *token, DIRLISTING_ON, DIRLISTING_OFF));
	}
	++token;
}

void ConfFile::_redirectionDirective(std::vector<ConfToken>::const_iterator &token,
                                     LocationBlock                          &location) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (2 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	try {
		uint16_t status = ft::stoi<uint16_t>(token->str());
		++token;
		location.setRedirect(status, token->str());
		++token;
	} catch (std::exception &e) {
		throw Configuration::ConfigurationException(this->_invalidValue(*directive, *token));
	}
}

void ConfFile::_indexDirective(std::vector<ConfToken>::const_iterator &token,
                               LocationBlock                          &location) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (0 == args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	for (uint32_t i = 0; i < args; ++i) {
		location.addIndex(token->str());
		++token;
	}
}

void ConfFile::_cgiDirective(std::vector<ConfToken>::const_iterator &token,
                             LocationBlock                          &location) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (2 > args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	const std::string binary = (token + args - 1)->str();
	for (uint32_t i = 0; i < args - 1; ++i) {
		location.addCGI(token->str(), binary);
		++token;
	}
	++token;
}

void ConfFile::_clientBodyTempPathDirective(std::vector<ConfToken>::const_iterator &token, LocationBlock &location) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	location.setClientBodyTempPath(token->str() + '/');
	++token;
}
