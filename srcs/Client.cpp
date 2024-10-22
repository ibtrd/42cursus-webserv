#include "Client.hpp"

#include <unistd.h>

Client::Client() : socket_fd(-1)
{
}

Client::Client(int socket_fd, struct sockaddr_in addr)
{
	this->socket_fd = socket_fd;
	this->addr = addr;
	this->addrlen = sizeof(addr);
}

Client::Client(const Client &src)
{
	*this = src;
}

Client::~Client()
{
	// close(this->socket_fd);
}

Client &Client::operator=(const Client &src)
{
	this->socket_fd = src.socket_fd;
	this->addr = src.addr;
	this->addrlen = src.addrlen;
	return *this;
}

void Client::closeSocket(void)
{
	close(this->socket_fd);
	this->socket_fd = -1;
}
