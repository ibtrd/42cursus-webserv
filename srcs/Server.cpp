
#include "Server.hpp"

#include <arpa/inet.h>
#include <sys/wait.h>

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
	for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin();
	     it != this->_fdClientMap.end(); ++it) {
		close(it->first);
	}
	for (std::list<Client>::const_iterator it = this->_clients.begin(); it != this->_clients.end();
	     ++it) {
		pid_t pid = it->cgiPid();
		if (pid != -1 && 0 == waitpid(pid, NULL, WNOHANG)) {
			kill(pid, SIGKILL);
		}
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

	// // DEBUG
	// for (servermap_t::const_iterator it = this->_serverBlocks.begin(); it !=
	// this->_serverBlocks.end(); ++it) { 	std::cerr << "\nServerBlock socket: " << it->first <<
	// std::endl; 	for (std::size_t i = 0; i < it->second.size(); ++i) {
	// 		// std::cerr << it->second[i] << std::endl;
	// 		for (std::size_t j = 0; j < it->second[i].locations().size(); ++j) {
	// 			std::cerr << it->second[i].locations()[j] << std::endl;
	// 		}
	// 	}
	// }

	// // DEBUG
	// std::cerr << "\nclientmap: " << this->_fdClientMap.size() << std::endl;
	// for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin(); it !=
	// this->_fdClientMap.end(); ++it) { 	std::cerr << "\t" << it->first << " -> " <<
	// it->second->socket() << std::endl;
	// }

	// // DEBUG
	// std::cerr << "\nclient list: " << this->_clients.size() << std::endl;
	// for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end();
	// ++it) { 	std::cerr << "\t" << it->socket() << std::endl;
	// }

	// // DEBUG
	// std::cerr << "\nmimetypes: " << this->_mimetypes.size() << std::endl;
	// for (mimetypes_t::const_iterator it = this->_mimetypes.begin(); it != this->_mimetypes.end();
	// ++it) { 	std::cerr << "\t" << it->first << " -> " << it->second << std::endl;
	// }

	// // DEBUG
	// std::cerr << "\ntimeouts: " << TIMEOUT_COUNT << std::endl;
	// for (uint32_t i = 0; i < TIMEOUT_COUNT; ++i) {
	// 	std::cerr << "\t" << i << " -> " << this->_timeouts[i] << std::endl;
	// }

	// // DEBUG
	// std::cerr << "\nepollFd: " << this->_epollFd << std::endl;

	// // DEBUG
	// std::cerr << "\nServer configured" << std::endl;
}

void Server::routine(void) {
	this->_nfds = epoll_wait(this->_epollFd, this->_events, MAX_EVENTS, EPOLL_WAIT_TIMEOUT);
	if (this->_nfds == -1) {
		if (g_signal != SIGQUIT) {
			std::cerr << "error: epoll_wait(): " << strerror(errno) << std::endl;
		}
		return;
	}

	// std::cerr << "\n\n--- New epoll_wait ---\n\n" << std::endl;
	// std::cerr << "nfds: " << this->_nfds << std::endl;

	// for (int32_t i = 0; i < this->_nfds; i++) {
	// 	std::cerr << "ring fd: " << this->_events[i].data.fd << std::endl;
	// }

	// std::cerr << std::endl;

	// // DEBUG
	// std::cerr << "\nclientmap after wait check: " << this->_fdClientMap.size() << std::endl;
	// for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin(); it !=
	// this->_fdClientMap.end();
	//      ++it) {
	// 	std::cerr << it->first << " -> " << it->second->socket() << std::endl;// << *it->second <<
	// std::endl;
	// }
	// std::cerr << "client list after wait check: " << this->_clients.size() << std::endl;
	// for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end();
	// ++it) { 	fd_t fds[2]; 	it->sockets(fds); 	std::cerr << it->socket() << " with cgi at " <<
	// fds[1]
	// << std::endl;
	// }
	// -----

	// Handle events
	for (int32_t i = 0; i < this->_nfds && 0 == g_signal; i++) {
		fd_t fd = this->_events[i].data.fd;

		// New connection
		if (this->_serverBlocks.find(fd) != this->_serverBlocks.end()) {
			if (this->_addConnection(fd)) {
				std::cerr << "Error adding connection" << std::endl;
			}
			continue;
		}

		// Existing connection
		clientbindmap_t::iterator it = this->_fdClientMap.find(fd);
		if (it == this->_fdClientMap.end()) {
			std::cerr << "No client for fd " << fd << std::endl;
			continue;
		}

		// Error events
		if (this->_events[i].events & (EPOLLERR | EPOLLHUP)) {
			std::cerr << "Close connection (EPOLLERR | EPOLLHUP) on fd " << fd << std::endl;
			if (this->_events[i].events & EPOLLERR) {
				std::cerr << "EPOLLERR" << std::endl;
			} else if (this->_events[i].events & EPOLLHUP) {
				std::cerr << "EPOLLHUP" << std::endl;
			}
			if (this->_removeConnection(fd) == SERVER_REMOVE) {
				continue;
			}
		}

		it = this->_fdClientMap.find(fd);
		if (it == this->_fdClientMap.end()) {
			std::cerr << "No client for read fd " << fd << std::endl;
			continue;
		}

		// Read events
		if (this->_events[i].events & EPOLLIN) {
			std::cerr << "Read event on fd " << fd << std::endl;

			error_t err = it->second->handleIn(fd);
			if (REQ_ERROR == err) {
				std::cerr << "Close connection (Error)" << std::endl;
				this->_removeConnection(fd);
				break;
			} else if (REQ_DONE == err) {
				std::cerr << "Close connection (Done)" << std::endl;
				this->_removeConnection(fd);
				break;
			}
		}

		it = this->_fdClientMap.find(fd);
		if (it == this->_fdClientMap.end()) {
			std::cerr << "No client for write fd " << fd << std::endl;
			continue;
		}

		// Write events
		if (this->_events[i].events & EPOLLOUT) {
			std::cerr << "Write event on fd " << fd << std::endl;

			error_t err = it->second->handleOut(fd);
			if (REQ_ERROR == err) {
				std::cerr << "Close connection (Error)" << std::endl;
				this->_removeConnection(fd);
				break;
			} else if (REQ_DONE == err) {
				std::cerr << "Close connection (Done)" << std::endl;
				this->_removeConnection(fd);
				break;
			}
		}
	}

	// // DEBUG
	// std::cerr << "\nclientmap pre timeout check: " << this->_fdClientMap.size() << std::endl;
	// for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin(); it !=
	// this->_fdClientMap.end();
	//      ++it) {
	// 	std::cerr << it->first << " -> " << it->second->socket() << std::endl;// << *it->second <<
	// std::endl;
	// }
	// std::cerr << "client list pre timeout check: " << this->_clients.size() << std::endl;
	// for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end();
	// ++it) { 	std::cerr << it->socket() << std::endl;
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
	// for (clientbindmap_t::const_iterator it = this->_fdClientMap.begin(); it !=
	// this->_fdClientMap.end();
	//      ++it) {
	// 	std::cerr << it->first << " -> " << it->second->socket() << std::endl;// << *it->second <<
	// std::endl;
	// }
	// std::cerr << "client list post timeout check: " << this->_clients.size() << std::endl;
	// for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end();
	// ++it) { 	std::cerr << it->socket() << std::endl;
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
	std::cerr << "Try adding cgi socket: " << socket << " to c" << client.socket() << std::endl;
	std::list<Client>::iterator itClient =
	    std::find(this->_clients.begin(), this->_clients.end(), client);
	if (itClient == this->_clients.end()) {
		std::cerr << "Error: addCGIToClientMap: client not found" << std::endl;
		return -1;
	}
	std::cerr << "Adding cgi socket: " << socket << " to c" << itClient->socket() << std::endl;
	this->_fdClientMap[socket] = itClient;
	return 0;
}

/* ************************************************************************** */

error_t Server::_addConnection(const int32_t socket) {  // TODO: REMOVE PRINTS
	struct sockaddr_in clientAddr;
	socklen_t          clientAddrLen = sizeof(clientAddr);

	errno                 = 0;
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

error_t Server::_removeConnection(const fd_t fd) {
	std::list<Client>::iterator client = this->_fdClientMap[fd];
	if (client == this->_clients.end()) {
		return (SERVER_REMOVE);
	}

	std::cerr << "Removing client: " << fd << std::endl;

	fd_t fds[2];
	client->sockets(fds);
	std::cerr << "fds: " << fds[0] << " " << fds[1] << std::endl;

	if (fd == fds[1]) {
		std::cerr << "CGI Hangup (to ignore)" << std::endl;
		return (SERVER_IGNORE_HANGUP);
	}

	if (fds[0] != -1) {
		errno = 0;
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fds[0], NULL)) {
			std::cerr << "fd: " << fds[0] << std::endl;
			throw std::runtime_error("epoll_ctl(): " + std::string(strerror(errno)));
		}
		std::cerr << "Removing fd: " << fds[0] << std::endl;
		this->_fdClientMap.erase(fds[0]);
		close(fds[0]);
	}
	if (fds[1] != -1) {
		errno = 0;
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fds[1], NULL)) {
			std::cerr << "fd: " << fds[1] << std::endl;
			throw std::runtime_error("epoll_ctl(): " + std::string(strerror(errno)));
		}
		std::cerr << "Removing fd: " << fds[1] << std::endl;
		this->_fdClientMap.erase(fds[1]);
		close(fds[1]);
		pid_t pid = client->cgiPid();
		if (pid != -1 && 0 == waitpid(pid, NULL, WNOHANG)) {
			std::cerr << "Killing CGI process: " << pid << std::endl;
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
		}
	}

	for (int i = 0; i < this->_nfds; ++i) {
		if (this->_events[i].data.fd == fds[0]) {
			this->_events[i].data.fd = -1;
			this->_events[i].events  = 0;
		}
		if (this->_events[i].data.fd == fds[1]) {
			this->_events[i].data.fd = -1;
			this->_events[i].events  = 0;
		}
	}

	std::cout << *client << std::endl;  // LOG
	this->_clients.erase(client);
	return (SERVER_REMOVE);
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
