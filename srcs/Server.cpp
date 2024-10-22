#include "Server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

Server::Server(/* args */)
{
}

Server::Server(int port)
{
	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverSocket < 0)
	{
		throw std::runtime_error("Error: socket creation failed");
	}

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

	if (listen(this->serverSocket, 1) < 0)
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
}

Server &Server::operator=(const Server &src)
{
	this->serverSocket = src.serverSocket;
	this->addr = src.addr;
	this->addrlen = src.addrlen;
	return *this;
}

void Server::acceptClient(void)
{
	struct sockaddr_in addr;
	socklen_t addrlen;
	
	int clientSocket = accept(this->serverSocket, (struct sockaddr *)&addr, &addrlen);
	if (clientSocket < 0)
	{
		throw std::runtime_error("Error: accept failed");
	}
	(void)addrlen;
	this->client = Client(clientSocket, addr);
	// this->clients.push_back(Client(clientSocket, this->addr));
}

void Server::dialog(void)
{
	ssize_t recv_size;
	char buffer[1024];

	std::cout << "Server dialog" << std::endl;
	while (1)
	{
		recv_size = recv(this->client.getSocketFd(), buffer, (1024 - 1), 0);
		if (recv_size < 0)
		{
			std::cout << "Client disconnected" << std::endl;
			break;
		}
		buffer[recv_size] = '\0';
		std::cout << "Client: " << buffer << " (" << recv_size << " bytes)" << "\n";
		std::cout << "Server: ";
		std::cin.getline(buffer, (1024 - 1));
		send(this->client.getSocketFd(), buffer, strlen(buffer), 0);
	}
}

void Server::closeClient(void)
{
	this->client.closeSocket();
}
