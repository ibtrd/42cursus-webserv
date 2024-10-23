#include "Server.hpp"

#include <string>
#include <stdexcept>
#include <cstring>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

Server::Server()
{
}

Server::Server(const Configuration &config)
{
	this->_serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (this->_serverSocket == -1)
	{
		std::string err = "Error: socket: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	// if (setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int)) == -1)
	// {
	// 	std::string err = "Error: setsockopt: ";
	// 	throw std::runtime_error((err + strerror(errno)).c_str());
	// }

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(config.port());
	socklen_t addrlen = sizeof(addr);

	if (bind(this->_serverSocket, (struct sockaddr *)&addr, addrlen) == -1)
	{
		std::string err = "Error: bind: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	if (listen(this->_serverSocket, config.backlog()) == -1)
	{
		std::string err = "Error: listen: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	this->_epollFd = epoll_create(1);
	if (this->_epollFd == -1)
	{
		std::string err = "Error: epoll_create: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = this->_serverSocket;
	if (epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, this->_serverSocket, &event) == -1)
	{
		std::string err = "Error: epoll_ctl: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}
}

Server::Server(const Server &src)
{
	(void)src;
}

Server::~Server()
{
	close(this->_serverSocket);
	close(this->_epollFd);
	for (std::map<int, t_client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		close(it->first);
	}
}

Server &Server::operator=(const Server &src)
{
	(void)src;
	return *this;
}

void Server::routine(void)
{
}

void Server::_init(void)
{
}
