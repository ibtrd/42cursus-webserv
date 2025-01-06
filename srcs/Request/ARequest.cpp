#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

#include "ARequest.hpp"

/* CONSTRUCTORS ************************************************************* */

// ARequest::ARequest(void)
// {
// 	// std::cerr << "ARequest created" << std::endl;
// }

ARequest::ARequest(Client &client) : _client(client)
{
	// std::cerr << "ARequest created" << std::endl;
}

ARequest::ARequest(const ARequest &other) : _client(other._client)
{
	// std::cerr << "ARequest copy" << std::endl;
	*this = other;
}

ARequest::~ARequest(void)
{
	// std::cerr << "ARequest destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

ARequest	&ARequest::operator=(const ARequest &other)
{
	std::cerr << "ARequest assign" << std::endl;
	if (this == &other)
		return (*this);
	return (*this);
}

/* ************************************************************************** */

/* ************************************************************************** */

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */
