#include "Server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

Server::Server(/* args */)
{
}

Server::Server(int port, int queueSize)
	// : queueSize(queueSize)
{
	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverSocket < 0)
	{
		throw std::runtime_error("Error: socket creation failed");
	}

	std::cerr << "Server socket: " << this->serverSocket << std::endl;

	int opt = 1;

	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		throw std::runtime_error("Error: setsockopt failed");
	}

	this->addrlen = sizeof(this->addr);

	this->addr.sin_family = AF_INET;
	this->addr.sin_addr.s_addr = INADDR_ANY;
	this->addr.sin_port = htons(port);

	if (bind(this->serverSocket, (struct sockaddr *)&this->addr, this->addrlen) < 0)
	{
		throw std::runtime_error(strerror(errno));
	}

	if (listen(this->serverSocket, queueSize) < 0)
	{
		throw std::runtime_error("Error: listen failed");
	}
}

Server::Server(const Server &src)
{
	*this = src;
}

Server::~Server()
{
	close(this->serverSocket);
	for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		it->closeSocket();
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
	int max_sd;
	int activity;
	int clientSocket;
	struct sockaddr_in addr;
	
	std::cout << "Server is running..." << std::endl;
	while (1)
	{
		FD_ZERO(&this->readfds);

		FD_SET(this->serverSocket, &this->readfds);
		max_sd = this->serverSocket;

		for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
		{
			// std::cerr << "Client socket_fd: " << it->getSocketFd() << std::endl;
			FD_SET(it->getSocketFd(), &this->readfds);
			if (it->getSocketFd() > max_sd)
				max_sd = it->getSocketFd();
		}

		// std::cerr << "max_sd: " << max_sd << std::endl;
		// std::cout << "selecting..." << std::endl;
		errno = 0;
		activity = select(max_sd + 1, &this->readfds, NULL, NULL, NULL);
		if ((activity < 0) && (errno != EINTR))
		{
			std::string err = "Error: select failed: ";
			err += strerror(errno);
			throw std::runtime_error(err.c_str());
		}

		if (FD_ISSET(this->serverSocket, &this->readfds))
		{
			// std::cout << "Server is ready" << std::endl;
			clientSocket = accept(this->serverSocket, (struct sockaddr *)&addr, &this->addrlen);
			if (clientSocket < 0)
			{
				throw std::runtime_error("Error: accept failed");
			}
			std::cout << "New client connected (" << inet_ntoa(addr.sin_addr) << ")" << std::endl;
			// std::cerr << "Client socket_fd: " << clientSocket << std::endl;

			this->clients.push_back(Client(clientSocket, addr));
		}

		for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end();)
		{
			if (FD_ISSET(it->getSocketFd(), &this->readfds))
			{
				char buffer[1024] = {0};
				int recv_size = recv(it->getSocketFd(), buffer, 1024 - 1, 0);
				if (recv_size < 0)
				{
					throw std::runtime_error("Error: recv failed");
				}
				else if (recv_size == 0)
				{
					std::cout << "Client disconnected (" << inet_ntoa(it->getAddr().sin_addr) << ")" << std::endl;
					it->closeSocket();
					it = this->clients.erase(it);
					continue;
				}
				else
				{
					std::string msg = inet_ntoa(it->getAddr().sin_addr);
					msg += ": ";
					msg += buffer;
					// std::cout << inet_ntoa(it->getAddr().sin_addr) << ": " << buffer << std::endl;
					std::cout << msg << std::endl;
					// send(it->getSocketFd(), buffer, strlen(buffer), 0);
					this->sendAll(msg.c_str());
				}
			}
			 ++it;
		}
	}
}

void Server::sendAll(const char *msg)
{
	for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		send(it->getSocketFd(), msg, strlen(msg), 0);
	}
}
