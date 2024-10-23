#include "Server.hpp"

Server::Server()
{
}

Server::Server(const Server &src)
{
	(void)src;
}

Server::~Server()
{
}

Server &Server::operator=(const Server &src)
{
	(void)src;
	return *this;
}
