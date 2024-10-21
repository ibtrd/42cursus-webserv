#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>

#include "Webserv.hpp"

/* CONSTRUCTORS ************************************************************* */

Webserv::Webserv(void) : _socket(-1) {}

Webserv::Webserv(const int32_t port) {
	this->_config.setPort(port);
	this->_init();
}

Webserv::Webserv(const std::string &configFile) {
	(void)configFile; //TODO configFile parsing 
	this->_init();
}

Webserv::~Webserv(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Webserv	&Webserv::operator=(const Webserv &other) {
	if (this == &other) {
		return (*this);
	}
	this->_config = other._config;
	this->_socket = other._socket;
	this->_address = other._address;
	return (*this);
}

/* PRIVATE METHODS ********************************************************** */

void	Webserv::_init(void) {
	this->_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_socket == -1) {
		std::string	message("socket(): ");
		message.append(strerror(errno));
		throw std::runtime_error(message);
	}
	this->_address.sin_family = AF_INET;
	this->_address.sin_port = htons(this->_config.port());
	this->_address.sin_addr.s_addr = INADDR_ANY;

	if (-1 == bind(this->_socket, (struct sockaddr*)&this->_address, sizeof(this->_address))) {
		std::string	message("bind(): ");
		message.append(strerror(errno));
		throw std::runtime_error(message);
	}
	if (-1 == listen(this->_socket, this->_config.backlog())) {
		std::string	message("listen(): ");
		message.append(strerror(errno));
		throw std::runtime_error(message);
	}
}

/* PUBLIC METHODS *********************************************************** */

void	Webserv::recieveMessage(void) const {
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
