#include "ServerBlock.hpp"

/* CONSTRUCTORS ************************************************************* */

ServerBlock::ServerBlock(void) {}

ServerBlock::ServerBlock(const ServerBlock &other) {
	*this = other;
}

ServerBlock::~ServerBlock(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

ServerBlock	&ServerBlock::operator=(const ServerBlock &other) {
	if (this == &other)
		return (*this);
	// Copy instructions
	return (*this);
}

/* ************************************************************************** */

/* GETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */
