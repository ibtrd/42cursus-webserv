
#include "Server.hpp"

#include <arpa/inet.h>
#include <sys/wait.h>

#include <csignal>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "ft.hpp"

extern int g_signal;

Server::Server() : _epollFd(-1) {}

Server::~Server() {
	if (this->_epollFd != -1) {
		close(this->_epollFd);
	}
	for (servermap_t::const_iterator it = this->_serverBlocks.begin();
	     it != this->_serverBlocks.end(); ++it) {
		close(it->first);
	}
	for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin(); it != this->_fdClientMap.end(); ++it) {
		close(it->first);
	}
	this->_serverBlocks.clear();
	this->_fdClientMap.clear();
	this->_mimetypes.clear();
	this->_clients.clear();
}

void Server::configure(const Configuration &config) {
	this->_epollFd = epoll_create(1);
	if (-1 == this->_epollFd) {
		throw std::runtime_error(std::string("epoll_create():") + strerror(errno));
	}
	Client::setEpollFd(this->_epollFd);

	socketbindmap_t                 bound;
	const std::vector<ServerBlock> &blocks = config.blocks();
	for (std::size_t i = 0; i < blocks.size(); ++i) {
		const std::vector<struct sockaddr_in> &hosts = blocks[i].hosts();
		for (std::size_t j = 0; j < hosts.size(); ++j) {
			socketbindmap_t::const_iterator it =
			    ft::isBound<fd_t, struct sockaddr_in>(bound, hosts[j]);
			if (bound.end() == it) {
				bound[this->_addSocket(blocks[i], hosts[j])].push_back(hosts[j]);
			} else {
				this->_serverBlocks[it->first].push_back(blocks[i]);
			}
		}
	}
	for (uint32_t type = CLIENT_HEADER_TIMEOUT; type != TIMEOUT_COUNT; ++type) {
		this->_timeouts[type] = config.timeout(type);
	}
	this->_mimetypes = config.mimetypes();
}

void Server::routine(void) {
	int32_t nfds = epoll_wait(this->_epollFd, this->_events, MAX_EVENTS, EPOLL_WAIT_TIMEOUT);
	if (nfds == -1) {
		if (g_signal != SIGQUIT) {
			std::cerr << "error: epoll_wait(): " << strerror(errno) << std::endl;
		}
		return;
	}

	std::cerr << "\n\n--- New epoll_wait ---\n\n" << std::endl;
	std::cerr << "nfds: " << nfds << std::endl;

	for (int32_t i = 0; i < nfds; i++) {
		std::cerr << "ring fd: " << this->_events[i].data.fd << std::endl;
	}

	std::cerr << std::endl;

	// DEBUG
	std::cerr << "\nclientmap after wait check: " << this->_fdClientMap.size() << std::endl;
	for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin(); it != this->_fdClientMap.end();
	     ++it) {
		std::cerr << it->first << " -> " << it->second->socket() << std::endl;// << *it->second << std::endl;
	}
	std::cerr << "client list after wait check: " << this->_clients.size() << std::endl;
	for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
		std::cerr << it->socket() << std::endl;
	}
	// -----

	// New connections and read events
	for (int32_t i = 0; i < nfds && 0 == g_signal; i++) {
		fd_t fd = this->_events[i].data.fd;

		if (this->_serverBlocks.find(fd) != this->_serverBlocks.end()) {
			if (this->_addConnection(fd)) {
				std::cerr << "Error adding connection" << std::endl;
			}
			continue;
		}
		if (this->_events[i].events & EPOLLIN) {
			std::cerr << "Read event on fd " << fd << std::endl;
			clientbindmap_t::iterator it = this->_fdClientMap.find(fd);
			if (it == this->_fdClientMap.end()) {
				std::cerr << "No client for fd " << fd << std::endl;
				continue;
			}
			error_t err = it->second->handleIn(fd);
			if (REQ_ERROR == err) {
				std::cerr << "Close connection (Error)" << std::endl;
				this->_removeConnection(fd);
			} else if (REQ_DONE == err) {
				std::cerr << "Close connection (Done)" << std::endl;
				this->_removeConnection(fd);
			}
		} else if (!(this->_events[i].events & EPOLLOUT)) {
			std::cerr << "Unknown event on fd " << fd << std::endl;
		}
	}

	// // DEBUG
	// std::cerr << "\nclientmap in|.|out check: " << this->_fdClientMap.size() << std::endl;
	// for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin(); it != this->_fdClientMap.end();
	//      ++it) {
	// 	std::cerr << it->first << " -> " << it->second->socket() << std::endl;// << *it->second << std::endl;
	// }
	// std::cerr << "client list in|.|out check: " << this->_clients.size() << std::endl;
	// for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
	// 	std::cerr << it->socket() << std::endl;
	// }
	// // -----

	// Write events
	for (int32_t i = 0; i < nfds && 0 == g_signal; i++) {
		fd_t fd = this->_events[i].data.fd;

		if (this->_events[i].events & EPOLLOUT) {
			std::cerr << "Write event on fd " << fd << std::endl;
			clientbindmap_t::iterator it = this->_fdClientMap.find(fd);

			if (it == this->_fdClientMap.end()) {
				std::cerr << "No client for fd " << fd << std::endl;
				continue;
			}
			error_t err = it->second->handleOut(fd);
			if (REQ_ERROR == err) {
				std::cerr << "Close connection (Error)" << std::endl;
				this->_removeConnection(fd);
			} else if (REQ_DONE == err) {
				std::cerr << "Close connection (Done)" << std::endl;
				this->_removeConnection(fd);
			}
		} else if (!(this->_events[i].events & EPOLLIN)) {
			std::cerr << "Unknown event on fd " << fd << std::endl;
		}
	}
	// // DEBUG
	// std::cerr << "\nclientmap pre timeout check: " << this->_fdClientMap.size() << std::endl;
	// for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin(); it != this->_fdClientMap.end();
	//      ++it) {
	// 	std::cerr << it->first << " -> " << it->second->socket() << std::endl;// << *it->second << std::endl;
	// }
	// std::cerr << "client list pre timeout check: " << this->_clients.size() << std::endl;
	// for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
	// 	std::cerr << it->socket() << std::endl;
	// }
	// // -----
	if (0 != g_signal) {
		while (this->_clients.size()) {
			this->_removeConnection(this->_clients.front().socket());
		}
		return;
	}
	this->_checkClientsTimeout();

	// // DEBUG
	// std::cerr << "\nclientmap post timeout check: " << this->_fdClientMap.size() << std::endl;
	// for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin(); it != this->_fdClientMap.end();
	//      ++it) {
	// 	std::cerr << it->first << " -> " << it->second->socket() << std::endl;// << *it->second << std::endl;
	// }
	// std::cerr << "client list post timeout check: " << this->_clients.size() << std::endl;
	// for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
	// 	std::cerr << it->socket() << std::endl;
	// }
	// // -----
}

const std::string &Server::getMimeType(const std::string &ext) const {
	static const std::string defaultMime(DEFAULT_MIMETYPE);

	mimetypes_t::const_iterator it = this->_mimetypes.find(ext);
	if (it == this->_mimetypes.end()) {
		return defaultMime;
	}
	return it->second;
}

const ServerBlock &Server::findServerBlock(const fd_t socket, const std::string &host) const {
	const std::vector<ServerBlock> &blocks = this->_serverBlocks.at(socket);
	for (uint32_t i = 0; i < blocks.size(); ++i) {
		const std::vector<std::string> &names = blocks[i].names();
		for (uint32_t j = 0; j < names.size(); ++j) {
			if (0 == host.compare(names[j])) {
				return blocks[i];
			}
		}
	}
	return blocks.front();
}

int32_t Server::getTimeout(const uint32_t type) const { return this->_timeouts[type]; }

fd_t Server::_addSocket(const ServerBlock &block, const struct sockaddr_in &host) {
	fd_t fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (-1 == fd) {
		throw std::runtime_error((std::string("socket(): ") + strerror(errno)).c_str());
	}
	this->_serverBlocks[fd] = std::vector<ServerBlock>();

	int reuse = 1;
	if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int))) {
		throw std::runtime_error((std::string("setsockopt(): ") + strerror(errno)).c_str());
	}

	socklen_t addrlen = sizeof(host);
	if (-1 == bind(fd, (struct sockaddr *)&host, addrlen)) {
		throw std::runtime_error((std::string("bind(): ") + strerror(errno)).c_str());
	}

	if (-1 == listen(fd, DEFAULT_BACKLOG)) {
		throw std::runtime_error((std::string("listen(): ") + strerror(errno)).c_str());
	}

	struct epoll_event event;
	event.events  = EPOLLIN;
	event.data.fd = fd;
	if (-1 == epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, fd, &event)) {
		throw std::runtime_error((std::string("epoll_ctl(): ") + strerror(errno)).c_str());
	}

	struct sockaddr_in boundAddr;
	socklen_t          boundAddrLen = sizeof(boundAddr);
	if (-1 == getsockname(fd, (struct sockaddr *)&boundAddr, &boundAddrLen)) {
		throw std::runtime_error((std::string("getsockname(): ") + strerror(errno)).c_str());
	}

	char ip[INET_ADDRSTRLEN];
	if (NULL == inet_ntop(AF_INET, &boundAddr.sin_addr, ip, sizeof(ip))) {
		throw std::runtime_error((std::string("inet_ntop(): ") + strerror(errno)).c_str());
	}

	std::cout << "Socket " << fd << " listening on " << ip << ":" << ntohs(boundAddr.sin_port)
	          << std::endl;
	this->_serverBlocks[fd].push_back(block);
	return fd;
}

error_t Server::addCGIToClientMap(const fd_t socket, const Client &client) {
	std::cerr << "Adding cgi socket: " << socket << std::endl;
	std::list<Client>::iterator itClient = std::find(this->_clients.begin(), this->_clients.end(), client);
	if (itClient == this->_clients.end()) {
		return -1;
	}
	this->_fdClientMap[socket] = itClient;
	return 0;
}

/* ************************************************************************** */

error_t Server::_addConnection(const int32_t socket) {  // TODO: REMOVE PRINTS
	struct sockaddr_in clientAddr;
	socklen_t          clientAddrLen = sizeof(clientAddr);

	errno = 0;
	int32_t requestSocket = accept(socket, (struct sockaddr *)&clientAddr, &clientAddrLen);
	if (-1 == requestSocket) {
		std::cerr << "Error: accept(): " << strerror(errno) << std::endl;
		return -1;
	}
	try {
		this->_clients.push_front(Client(socket, requestSocket, *this, clientAddr));
	} catch (...) {
		close(requestSocket);
		return -1;
	}
	if (-1 == this->_clients.front().init()) {
		return -1;
	}
	this->_fdClientMap[requestSocket] = this->_clients.begin();
	return 0;
}

void Server::_removeConnection(const fd_t fd) {
	std::list<Client>::iterator client = this->_fdClientMap[fd];
	if (client == this->_clients.end()) {
		return;
	}

	std::cerr << "Removing client: " << fd << std::endl;


	// // DEBUG
	// std::cerr << "\nclientmap on remove check: " << this->_fdClientMap.size() << std::endl;
	// for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin(); it != this->_fdClientMap.end();
	//      ++it) {
	// 	std::cerr << it->first << " -> " << it->second->socket() << std::endl;// << *it->second << std::endl;
	// }
	// std::cerr << "client list on remove check: " << this->_clients.size() << std::endl;
	// for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
	// 	std::cerr << it->socket() << std::endl;
	// }
	// // -----

	fd_t fds[2];
	client->sockets(fds);
	// for (int i = 0; i < 2; ++i) {
	// 	if (fds[i] != -1) {
	// 		struct epoll_event event;
	// 		errno = 0;
	// 		if (epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fds[i], &event)) {
	// 			std::cerr << "Error: epoll_ctl(): " << strerror(errno) << std::endl;
	// 			std::cerr << i << ". fd: " << fds[i] << std::endl;
	// 			throw std::runtime_error("epoll_ctl(): " + std::string(strerror(errno)));
	// 		}
	// 		this->_fdClientMap.erase(fds[i]);
	// 		close(fds[i]);
	// 	}
	// }
	if (fds[0] != -1) {
		struct epoll_event event;
		errno = 0;
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fds[0], &event)) {
			std::cerr << "Error: epoll_ctl(): " << strerror(errno) << std::endl;
			std::cerr << "fd: " << fds[0] << std::endl;
		}
		this->_fdClientMap.erase(fds[0]);
		close(fds[0]);
	}
	if (fds[1] != -1) {
		struct epoll_event event;
		errno = 0;
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fds[1], &event)) {
			std::cerr << "Error: epoll_ctl(): " << strerror(errno) << std::endl;
			std::cerr << "fd: " << fds[1] << std::endl;
			// throw std::runtime_error("epoll_ctl(): " + std::string(strerror(errno)));
		}
		this->_fdClientMap.erase(fds[1]);
		close(fds[1]);
		pid_t pid = client->cgiPid();
		if (pid != -1 && 0 == waitpid(pid, NULL, WNOHANG)) {
			std::cerr << "Killing CGI process: " << pid << std::endl;
			kill(pid, SIGINT);
		}
	}

	std::cout << *client << std::endl;	// LOG
	this->_clients.erase(client);
}

void Server::_checkClientsTimeout(void) {
	const time_t now = time(NULL);

	for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end();) {
		std::list<Client>::iterator tmp = it++;
		switch (tmp->timeoutCheck(now)) {
			case REQ_DONE:
				std::cerr << "Done: timeoutCheck" << std::endl;
				this->_removeConnection(tmp->socket());
				break;

			case REQ_ERROR:
				std::cerr << "Error: timeoutCheck" << std::endl;
				this->_removeConnection(tmp->socket());
				break;

			default:
				break;
		}
	}
}

int32_t Server::epollFd(void) const { return (this->_epollFd); }
