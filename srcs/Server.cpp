#include "Server.hpp"
#include "ft.hpp"

#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <algorithm>
#include <csignal>
#include <fstream>

#include "Server.hpp"
#include "ft.hpp"

extern int g_signal;

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
	Client::setEpollFd(this->_epollFd);

	bindmap_t	bound;
	const std::vector<ServerBlock> &blocks = config.blocks();
	for (std::size_t i = 0; i < blocks.size(); ++i) {
		const std::vector<struct sockaddr_in> &hosts = blocks[i].hosts();
		for (std::size_t j = 0; j < hosts.size(); ++j) {
			bindmap_t::const_iterator it = ft::isBound<fd_t, struct sockaddr_in>(bound, hosts[j]);
			if (bound.end() == it) {
				bound[this->_addSocket(blocks[i], hosts[j])].push_back(hosts[j]);
			} else {
				this->_serverBlocks[it->first].push_back(blocks[i]);
			}
		}
	}

	if (-1 == this->_loadMimeTypes()) {
		throw std::runtime_error("Failed to load mime types");
	}
}

void Server::routine(void) {
	int32_t nfds = epoll_wait(this->_epollFd, this->_events, MAX_EVENTS, 50000);
	if (nfds == -1) {
		if (g_signal != SIGQUIT)
			std::cerr << "error: epoll_wait(): " << strerror(errno) << std::endl;
		return ;
	}
	for(int i = 0; i < nfds; i++) {
      int32_t fd = this->_events[i].data.fd;
		if (this->_serverBlocks.find(fd) != this->_serverBlocks.end()) {
			this->_addConnection(fd);
		} else {
			switch (this->_clients[fd].handle())
			{
			case REQ_ERROR:
				std::cerr << "Close connection (Error)" << std::endl;
				this->_removeConnection(fd);
				break;

			case REQ_DONE:
				std::cerr << "Close connection (Done)" << std::endl;
				this->_removeConnection(fd);
				break;

			default:
				break;
			}
		}
	}
}

const std::string &Server::getMimeType(const std::string &ext) const {
	std::map<std::string, std::string>::const_iterator it = this->_mimetypes.find(ext);
	if (it == this->_mimetypes.end()) {
		return this->_mimetypes.at("default");
	}
	return it->second;
}

fd_t Server::_addSocket(const ServerBlock &block, const struct sockaddr_in &host) {
	fd_t fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (-1 == fd) {
		throw std::runtime_error((std::string("socket(): ") + strerror(errno)).c_str());
	}
	this->_serverBlocks[fd] = std::vector<ServerBlock>();

	int reuse = 1;
 	if(-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int))) {
		throw std::runtime_error((std::string("setsockopt(): ") + strerror(errno)).c_str());
	}

	socklen_t addrlen = sizeof(host);
	if (-1 == bind(fd, (struct sockaddr *)&host, addrlen)){
		throw std::runtime_error((std::string("bind(): ") + strerror(errno)).c_str());
	}

	if (-1 == listen(fd, DEFAULT_BACKLOG)) {
		throw std::runtime_error((std::string("listen(): ") + strerror(errno)).c_str());
	}

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fd;
	if (-1 == epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, fd, &event)) {
		throw std::runtime_error((std::string("epoll_ctl(): ") + strerror(errno)).c_str());
	}

	struct sockaddr_in boundAddr;
	socklen_t boundAddrLen = sizeof(boundAddr);
	if (-1 == getsockname(fd, (struct sockaddr *)&boundAddr, &boundAddrLen)) {
		throw std::runtime_error((std::string("getsockname(): ") + strerror(errno)).c_str());
	}

	char ip[INET_ADDRSTRLEN];
	if (NULL == inet_ntop(AF_INET, &boundAddr.sin_addr, ip, sizeof(ip))) {
		throw std::runtime_error((std::string("inet_ntop(): ") + strerror(errno)).c_str());
	}

	std::cout << "Socket " << fd << " listening on " << ip << ":" << ntohs(boundAddr.sin_port) << std::endl;
	this->_serverBlocks[fd].push_back(block);
	return fd;
}

error_t	Server::_addConnection(const int32_t socket) {
	std::cerr << "New connection" << std::endl;
	int32_t requestSocket = accept(socket, NULL, NULL);
	if (-1 == requestSocket) {
		return -1;
	}
	if (-1 == this->_clients[requestSocket].init(requestSocket, &this->_serverBlocks.at(socket))) {
		return -1;
	}
	return 0;
}

void Server::_removeConnection(const int32_t socket) {
	this->_clients.erase(socket);
	close(socket);
	epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, socket, NULL);
}

error_t Server::_loadMimeTypes(void)
{
	std::ifstream file(MIME_TYPE_FILE);
	if (!file.is_open()) {
		return -1;
	}
	std::string line;
	while (std::getline(file, line)) {
		if (line[0] == '#' || line.empty()) {
			continue;
		}
		std::istringstream iss(line);
		std::string type;
		iss >> type;
		while (iss) {
			std::string ext;
			iss >> ext;
			if (ext.empty()) {
				break;
			}
			this->_mimetypes[ext] = type;
		}
	}
	this->_mimetypes["default"] = "application/octet-stream";
	// DEBUG
	// for (std::map<std::string, std::string>::const_iterator it = this->_mimetypes.begin(); it != this->_mimetypes.end(); ++it) {
	// 	std::cout << it->first << " -> " << it->second << std::endl;
	// }
	return 0;
}

int32_t	Server::epollFd(void) const {
	return (this->_epollFd);
}
