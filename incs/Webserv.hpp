#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <arpa/inet.h>

# include "Configuration.hpp"

class Webserv {
private:
	Configuration	_config;
	int32_t			_socket;
	sockaddr_in		_address;

	void	_init(void);

public:
	Webserv(void);
	Webserv(const int32_t port);
	Webserv(const std::string &configFile);

	~Webserv(void);

	Webserv	&operator=(const Webserv &other);

	void	recieveMessage(void) const;

	int32_t	closeSocket(void);

};

#endif /* ******************************************************************* */
