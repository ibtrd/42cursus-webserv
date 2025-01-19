#include "ConfFile.hpp"
#include "Configuration.hpp"
#include "ft.hpp"

void ConfFile::_clientHeaderTimeoutDirective(std::vector<ConfToken>::const_iterator &token) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	try {
		errno            = 0;
		int32_t duration = ft::stoi<int32_t>(token->str());
		if (duration < 0) {
			throw std::invalid_argument("Invalid value");
		}
		this->timeouts[CLIENT_HEADER_TIMEOUT] = duration;
	} catch (std::exception &e) {
		throw Configuration::ConfigurationException(this->_invalidValue(*directive, *token));
	}
	++token;
}

void ConfFile::_clientBodyTimeoutDirective(std::vector<ConfToken>::const_iterator &token) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	try {
		errno            = 0;
		int32_t duration = ft::stoi<int32_t>(token->str());
		if (duration < 0) {
			throw std::invalid_argument("Invalid value");
		}
		this->timeouts[CLIENT_BODY_TIMEOUT] = duration;
	} catch (std::exception &e) {
		throw Configuration::ConfigurationException(this->_invalidValue(*directive, *token));
	}
	++token;
}

void ConfFile::_sendTimeoutDirective(std::vector<ConfToken>::const_iterator &token) {
	const uint32_t                               args      = this->_countArgs(token);
	const std::vector<ConfToken>::const_iterator directive = token++;

	if (1 != args) {
		throw Configuration::ConfigurationException(this->_invalidArgumentNumber(*directive));
	}
	try {
		errno            = 0;
		int32_t duration = ft::stoi<int32_t>(token->str());
		if (duration < 0) {
			throw std::invalid_argument("Invalid value");
		}
		this->timeouts[SEND_TIMEOUT] = duration;
	} catch (std::exception &e) {
		throw Configuration::ConfigurationException(this->_invalidValue(*directive, *token));
	}
	++token;
}
