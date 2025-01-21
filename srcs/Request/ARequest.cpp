#include <iostream>

#include "Client.hpp"

/* CONSTRUCTORS ************************************************************* */

// ARequest::ARequest(void)
// {
// 	// std::cerr << "ARequest created" << std::endl;
// }

ARequest::ARequest(RequestContext_t &context) : _context(context) {
	// std::cerr << "ARequest created" << std::endl;
	Path target(this->_context.target);
	target      = target.subPath(this->_context.ruleBlock->path().prefixLength(), SIZE_MAX);
	this->_path = this->_context.ruleBlock->getRoot().concat(target);
}

ARequest::ARequest(const ARequest &other) : _context(other._context) {
	// std::cerr << "ARequest copy" << std::endl;
	Path target(this->_context.target);
	target      = target.subPath(this->_context.ruleBlock->path().prefixLength(), SIZE_MAX);
	this->_path = this->_context.ruleBlock->getRoot().concat(target);
}

ARequest::~ARequest(void) {
	// std::cerr << "ARequest destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

ARequest &ARequest::operator=(const ARequest &other) {
	std::cerr << "ARequest assign" << std::endl;
	if (this == &other) {
		return (*this);
	}
	// this->_context = other._context;
	return (*this);
}

/* ************************************************************************** */

/* ************************************************************************** */

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */
