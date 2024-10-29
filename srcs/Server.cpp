#include "Server.hpp"

#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

Server::Server()
{
}

Server::~Server()
{
	close(this->_serverSocket);
	close(this->_epollFd);
	// for (std::map<int, t_client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	// {
	// 	close(it->first);
	// }
}

void	Server::configure(const Configuration &config) {
	this->_serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (this->_serverSocket == -1)
	{
		std::string err = "Error: socket: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	int reuse = 1;
 	if(setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
		std::string err = "Error: setsockopt: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

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
	Request::setEpollFd(this->_epollFd);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = this->_serverSocket;
	if (epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, this->_serverSocket, &event) == -1)
	{
		std::string err = "Error: epoll_ctl: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}
}

void Server::routine(void)
{
	int32_t nfds = epoll_wait(this->_epollFd, this->_events, MAX_EVENTS, 50000);
	if (nfds == -1) {
		std::cerr << "error: epoll_wait: " << strerror(errno) << std::endl;
		return ;
	}
	for(int i = 0; i < nfds; i++) {
      int32_t fd = this->_events[i].data.fd;
		if (fd == this->_serverSocket) {
			this->_addConnection(fd);
		} else {
			if (this->_requests[fd].handleRequest()) {
				this->_requests.erase(fd);
				close(fd);
			}
			// this->_requests.erase(fd);
			// close(fd);
		}
    }
}

void Server::_init(void)
{
}

void	Server::_addConnection(const int32_t socket) {
	try {
		Request	req(socket);
		this->_requests[req.socket()] = req;
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

int32_t	Server::epollFd(void) const { return (this->_epollFd); }
