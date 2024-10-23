#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <sys/select.h>
#include <fcntl.h>

#include "Server.hpp"

/* CONSTRUCTORS ************************************************************* */

Server::Server(void) : _socket(-1), _maxfd(0) {}

Server::Server(const int32_t port) : _maxfd(0) {
	this->_config.setPort(port);
	this->_init();
}

Server::Server(const std::string &configFile) : _maxfd(0) {
	(void)configFile; //TODO configFile parsing 
	this->_init();
}

Server::~Server(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Server	&Server::operator=(const Server &other) {
	if (this == &other) {
		return (*this);
	}
	this->_config = other._config;
	this->_socket = other._socket;
	this->_maxfd = other._maxfd;
	this->_address = other._address;
	this->_clientsSockets = other._clientsSockets;
	return (*this);
}

/* PRIVATE METHODS ********************************************************** */

void	Server::_init(void) {
	this->_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_socket == -1) {
		std::string	message("socket(): ");
		message.append(strerror(errno));
		throw std::runtime_error(message);
	}
	if (this->_socket > this->_maxfd) {
		this->_maxfd = this->_socket;
	}
	FD_ZERO(&this->_clientsSockets);
	FD_SET(this->_socket, &this->_clientsSockets);

	this->_address.sin_family = AF_INET;
	this->_address.sin_port = htons(this->_config.port());
	this->_address.sin_addr.s_addr = INADDR_ANY;

	const int enable = 1;
	if (setsockopt(this->_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    	std::string	message("bind(): ");
		message.append(strerror(errno));
		std::cerr << message;
	}

	if (0 > bind(this->_socket, (struct sockaddr*)&this->_address, sizeof(this->_address))) {
		std::string	message("bind(): ");
		message.append(strerror(errno));
		throw std::runtime_error(message);
	}
	if (0 > listen(this->_socket, this->_config.backlog())) {
		std::string	message("listen(): ");
		message.append(strerror(errno));
		throw std::runtime_error(message);
	}
}

void	Server::_addConnection(void) {
	// struct machin test;

	std::cout << "adding client" << std::endl;
	int32_t	clientSocket = accept(this->_socket, NULL, NULL);
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK | FD_CLOEXEC)) {
		std::cerr << "fcntl" << std::endl;
		throw std::runtime_error(strerror(errno));
	}
	if (-1 == clientSocket) {
		std::cerr << "error accpept" << std::endl;
		throw std::runtime_error(strerror(errno));
	}
	FD_SET(clientSocket, &this->_clientsSockets);
	if (clientSocket > this->_maxfd) {
		this->_maxfd = clientSocket;
		std::cout << "new max fd: " << this->_maxfd << std::endl;
	}
	// char client_ip[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, &test.addr.sin_addr, client_ip, sizeof(client_ip));

	std::cout << "New client!";
}

/* PUBLIC METHODS *********************************************************** */

int32_t	Server::closeSocket(void) {
	int32_t status = close(this->_socket);
	this->_socket = -1;
	return status;
}

void	Server::routine(void) {
	fd_set fds = this->_clientsSockets;
	select(FD_SETSIZE, &fds, NULL, NULL, NULL);
	for (int i = 0; i < FD_SETSIZE; ++i) {
		if (FD_ISSET(i, &fds)) {
				std::cerr << "socket " << i << "is set!" << std::endl;
			if (i == this->_socket) {
				this->_addConnection();
			}  else {
				char buffer[1024] = {0};
				std::cout << "ce fd est ready: " << i << std::endl;
				if (recv(i, buffer, sizeof(buffer), 0) > 0) {
					std::cerr << "recv error" << std::endl;
				}
				std::cout << "Message from client: " << buffer << std::endl;
			}
		}
	}
}

/* GETTERS ****************************************************************** */



/* EXCEPTIONS *************************************************************** */
