#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <arpa/inet.h>
# include <map>

# include "Configuration.hpp"

class Webserv {
private:
	Configuration	_config;
	int32_t			_socket;
	int32_t			_maxfd;
	fd_set			_clientsSockets;
	sockaddr_in		_address;

	void	_init(void);
	void	_addConnection(void);

public:
	Webserv(void);
	Webserv(const int32_t port);
	Webserv(const std::string &configFile);

	~Webserv(void);

	Webserv	&operator=(const Webserv &other);

	void	recieveMessage(void) const;
	void	routine(void);

	int32_t	closeSocket(void);

};

#endif /* ******************************************************************* */
