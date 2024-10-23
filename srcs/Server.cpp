#include "Server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <algorithm>

Server::Server(/* args */)
{
}

Server::Server(int port, int queueSize)
	// : queueSize(queueSize)
{
	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverSocket < 0)
	{
		std::string err = "Error: socket: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	std::cerr << "Server socket: " << this->serverSocket << std::endl;

	int opt = 1;

	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		std::string err = "Error: setsockopt: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	this->addrlen = sizeof(this->addr);

	this->addr.sin_family = AF_INET;
	this->addr.sin_addr.s_addr = INADDR_ANY;
	this->addr.sin_port = htons(port);

	if (bind(this->serverSocket, (struct sockaddr *)&this->addr, this->addrlen) < 0)
	{
		std::string err = "Error: bind: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	if (listen(this->serverSocket, queueSize) < 0)
	{
		std::string err = "Error: listen: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	this->epoll_fd = epoll_create1(0);
	if (this->epoll_fd < 0)
	{
		std::string err = "Error: epoll_create1: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	struct epoll_event ev;
	
	ev.events = EPOLLIN;
	ev.data.fd = this->serverSocket;
	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->serverSocket, &ev) < 0)
	{
		std::string err = "Error: epoll_ctl: ";
		throw std::runtime_error((err + strerror(errno)).c_str());
	}

	std::cout << "Listening on port " << port << std::endl;
}

Server::Server(const Server &src)
{
	*this = src;
}

Server::~Server()
{
	close(this->serverSocket);
	for (std::vector<int>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		close(*it);
	}
}

Server &Server::operator=(const Server &src)
{
	this->serverSocket = src.serverSocket;
	this->addr = src.addr;
	this->addrlen = src.addrlen;
	return *this;
}

void Server::run(void)
{
	int clientSocket;
	struct sockaddr_in addr;
	struct epoll_event ev;
	
	std::cout << "Server is running..." << std::endl;
	while (1)
	{
		int nfds = epoll_wait(this->epoll_fd, this->events, MAX_EVENTS, -1);
		if (nfds == -1)
		{
			std::string err = "Error: epoll_wait: ";
			throw std::runtime_error((err + strerror(errno)).c_str());
		}

		for (int n = 0; n < nfds; n++)
		{
			if (events[n].data.fd == this->serverSocket)
			{
				clientSocket = accept(this->serverSocket, (struct sockaddr *)&addr, &this->addrlen);
				if (clientSocket < 0)
				{
					std::string err = "Error: accept: ";
					throw std::runtime_error((err + strerror(errno)).c_str());
				}
				std::cout << "New client connected (" << inet_ntoa(addr.sin_addr) << ")" << std::endl;
				ev.events = EPOLLIN;
				ev.data.fd = clientSocket;
				if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, clientSocket, &ev) < 0)
				{
					std::string err = "Error: epoll_ctl: ";
					throw std::runtime_error((err + strerror(errno)).c_str());
				}
				this->clients.push_back(clientSocket);
			}
			else
			{
				clientSocket = events[n].data.fd;
				getpeername(clientSocket, (struct sockaddr *)&addr, &this->addrlen);
				char buffer[1024] = {0};
				int recv_size = recv(clientSocket, buffer, 1024 - 1, 0);
				if (recv_size < 0)
				{
					std::string err = "Error: recv: ";
					throw std::runtime_error((err + strerror(errno)).c_str());
				}
				else if (recv_size == 0)
				{
					std::cout << "Client disconnected (" << inet_ntoa(addr.sin_addr) << ")" << std::endl;
					ev.events = EPOLLIN;
					ev.data.fd = clientSocket;
					if (epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, clientSocket, &ev) < 0)
					{
						std::string err = "Error: epoll_ctl: ";
						throw std::runtime_error((err + strerror(errno)).c_str());
					}
					close(clientSocket);
					this->clients.erase(std::remove(this->clients.begin(), this->clients.end(), clientSocket), this->clients.end());
				}
				else
				{
					std::string msg = inet_ntoa(addr.sin_addr);
					msg += ": ";
					msg += buffer;
					std::cout << msg << std::endl;
					this->sendAll(msg.c_str());
				}
			}
		}

		// for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
		// {
		// 	// std::cerr << "Client socket_fd: " << it->getSocketFd() << std::endl;
		// 	FD_SET(it->getSocketFd(), &this->readfds);
		// 	if (it->getSocketFd() > max_sd)
		// 		max_sd = it->getSocketFd();
		// }

		// // std::cerr << "max_sd: " << max_sd << std::endl;
		// // std::cout << "selecting..." << std::endl;
		// errno = 0;
		// activity = select(max_sd + 1, &this->readfds, NULL, NULL, NULL);
		// if ((activity < 0) && (errno != EINTR))
		// {
		// 	std::string err = "Error: select: ";
		// 	throw std::runtime_error((err + strerror(errno)).c_str());
		// }

		// if (FD_ISSET(this->serverSocket, &this->readfds))
		// {
		// 	// std::cout << "Server is ready" << std::endl;
		// 	clientSocket = accept(this->serverSocket, (struct sockaddr *)&addr, &this->addrlen);
		// 	if (clientSocket < 0)
		// 	{
		// 		std::string err = "Error: accept: ";
		// 		throw std::runtime_error((err + strerror(errno)).c_str());
		// 	}
		// 	std::cout << "New client connected (" << inet_ntoa(addr.sin_addr) << ")" << std::endl;
		// 	// std::cerr << "Client socket_fd: " << clientSocket << std::endl;

		// 	this->clients.push_back(Client(clientSocket, addr));
		// }

		// for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end();)
		// {
		// 	if (FD_ISSET(it->getSocketFd(), &this->readfds))
		// 	{
		// 		char buffer[1024] = {0};
		// 		int recv_size = recv(it->getSocketFd(), buffer, 1024 - 1, 0);
		// 		if (recv_size < 0)
		// 		{
		// 			std::string err = "Error: recv: ";
		// 			throw std::runtime_error((err + strerror(errno)).c_str());
		// 		}
		// 		else if (recv_size == 0)
		// 		{
		// 			std::cout << "Client disconnected (" << inet_ntoa(it->getAddr().sin_addr) << ")" << std::endl;
		// 			it->closeSocket();
		// 			it = this->clients.erase(it);
		// 			continue;
		// 		}
		// 		else
		// 		{
		// 			std::string msg = inet_ntoa(it->getAddr().sin_addr);
		// 			msg += ": ";
		// 			msg += buffer;
		// 			// std::cout << inet_ntoa(it->getAddr().sin_addr) << ": " << buffer << std::endl;
		// 			std::cout << msg << std::endl;
		// 			// send(it->getSocketFd(), buffer, strlen(buffer), 0);
		// 			this->sendAll(msg.c_str());
		// 		}
		// 	}
		// 	 ++it;
		// }
	}
}

void Server::sendAll(const char *msg)
{
	for (std::vector<int>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		if (send(*it, msg, strlen(msg), 0) == -1)
		{
			std::string err = "Error: send: ";
			throw std::runtime_error((err + strerror(errno)).c_str());
		}
	}
}
