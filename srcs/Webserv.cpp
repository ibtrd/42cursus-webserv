#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>

#include "Webserv.hpp"

/* CONSTRUCTORS ************************************************************* */

Webserv::Webserv(void) :_socket(-1), _maxQueue(5) {}

Webserv::Webserv(const int32_t port) : _maxQueue(5) {
	this->_init(port);
}

Webserv::Webserv(const int32_t port, const int32_t maxQueue) : _maxQueue(maxQueue) {
	this->_init(port);
}

Webserv::~Webserv(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Webserv	&Webserv::operator=(const Webserv &other) {
	if (this == &other) {
		return (*this);
	}
	this->_socket = other._socket;
	this->_address = other._address;
	this->_maxQueue = other._maxQueue;
	return (*this);
}

/* PRIVATE METHODS ********************************************************** */

void	Webserv::_init(const int32_t port) {
	this->_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_socket == -1) {
		std::string	message("socket(): ");
		message.append(strerror(errno));
		throw std::runtime_error(message);
	}
	this->_address.sin_family = AF_INET;
	this->_address.sin_port = htons(port);
	this->_address.sin_addr.s_addr = INADDR_ANY;

	if (-1 == bind(this->_socket, (struct sockaddr*)&this->_address, sizeof(this->_address))) {
		std::string	message("bind(): ");
		message.append(strerror(errno));
		throw std::runtime_error(message);
	}
	if (-1 == listen(this->_socket, 5)) {
		std::string	message("listen(): ");
		message.append(strerror(errno));
		throw std::runtime_error(message);
	}
}

/* PUBLIC METHODS *********************************************************** */

void	Webserv::recieveMessage(void) const {
	std::cerr << this->_socket << std::endl;
	int clientSocket = accept(this->_socket, NULL, NULL);
	if (-1 == clientSocket) {
		throw std::runtime_error(strerror(errno));
	}

	char buffer[1024] = {0};
	recv(clientSocket, buffer, sizeof(buffer), 0);
	if (close(clientSocket)) {
		std::cerr << "close error" << std::endl;
	}
	std::cout << "Message from client: " << buffer << std::endl;
}

int32_t	Webserv::closeSocket(void) {
	int32_t status = close(this->_socket);
	this->_socket = -1;
	return status;
}

/* GETTERS ****************************************************************** */



/* EXCEPTIONS *************************************************************** */
