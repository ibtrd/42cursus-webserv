#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <algorithm>

#include "Server.hpp"
#include "ft.hpp"

Server::Server() : _epollFd(-1) {}

Server::~Server() {
	if (this->_epollFd != -1) {
		close(this->_epollFd);
	}
	for (servermap_t::const_iterator it = this->_serverBlocks.begin(); it != this->_serverBlocks.end(); ++it) {
		close(it->first);
	}
}

void Server::configure(const Configuration &config) {
	this->_epollFd = epoll_create(1);
	if (-1 == this->_epollFd) {
		std::string err = "epoll_create: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}
	Request::setEpollFd(this->_epollFd);

	bindmap_t	bound;
	const std::vector<ServerBlock> &blocks = config.blocks();
	for (std::size_t i = 0; i < blocks.size(); ++i) {
		const std::vector<struct sockaddr_in> &hosts = blocks[i].getHosts();
		for (std::size_t j = 0; j < hosts.size(); ++j) {
			bindmap_t::const_iterator it = ft::isBound<fd_t, struct sockaddr_in>(bound, hosts[j]);
			if (bound.end() == it) {
				bound[this->_addSocket(blocks[i], hosts[j])].push_back(hosts[j]);
			} else {
				this->_serverBlocks[it->first].push_back(blocks[i]);
			}
		}
	}
}

void Server::routine(void) {
	int32_t nfds = epoll_wait(this->_epollFd, this->_events, MAX_EVENTS, 50000);
	if (nfds == -1) {
		std::cerr << "error: epoll_wait: " << strerror(errno) << std::endl;
		return ;
	}
	for(int i = 0; i < nfds; i++) {
      int32_t fd = this->_events[i].data.fd;
		if (this->_serverBlocks.find(fd) != this->_serverBlocks.end()) {
			this->_addConnection(fd);
		} else {
			if (this->_requests[fd].handle()) {
				std::cerr << "Close connection" << std::endl;
				this->_removeConnection(fd);
			}
		}
	}
}

fd_t Server::_addSocket(const ServerBlock &block, const struct sockaddr_in &host) {
	fd_t fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (-1 == fd) {
		std::string err = "socket(): ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}
	this->_serverBlocks[fd] = std::vector<ServerBlock>();

	int reuse = 1;
 	if(-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int))) {
		std::string err = "setsockopt(): ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	socklen_t addrlen = sizeof(host);
	if (-1 == bind(fd, (struct sockaddr *)&host, addrlen)){
		std::string err = "bind(): ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	if (-1 == listen(fd, DEFAULT_BACKLOG)) {
		std::string err = "Error: listen: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (-1 == epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, fd, &event)) {
		std::string err = "epoll_ctl(): ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	struct sockaddr_in boundAddr;
	socklen_t boundAddrLen = sizeof(boundAddr);
	if (-1 == getsockname(fd, (struct sockaddr *)&boundAddr, &boundAddrLen)) {
		std::string err = "getsockname(): ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	char ip[INET_ADDRSTRLEN];
	if (NULL == inet_ntop(AF_INET, &boundAddr.sin_addr, ip, sizeof(ip))) {
		std::string err = "inet_ntop(): ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	std::cout << "Socket " << fd << " listening on " << ip << ":" << ntohs(boundAddr.sin_port) << std::endl;
	this->_serverBlocks[fd].push_back(block);
	return fd;
}

error_t	Server::_addConnection(const int32_t socket) {
	std::cerr << "New connection" << std::endl;
	int32_t requestSocket = accept(socket, NULL, NULL);
	if (-1 == requestSocket) {
		return (-1);
	}
	if (-1 == this->_requests[requestSocket].init(requestSocket)) {
		return -1;
	}
	return 0;
}

void Server::_removeConnection(const int32_t socket) {
	this->_requests.erase(socket);
	close(socket);
	epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, socket, NULL);
}

int32_t	Server::epollFd(void) const {
	return (this->_epollFd);
}
