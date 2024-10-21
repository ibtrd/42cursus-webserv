#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <arpa/inet.h>

class Webserv {
private:
	int32_t		_socket;
	sockaddr_in	_address;
	int32_t		_maxQueue;

	void	_init(const int32_t port);

public:
	Webserv(void);
	Webserv(const int32_t port);
	Webserv(const int32_t port, const int32_t maxQueue);

	~Webserv(void);

	Webserv	&operator=(const Webserv &other);

	void	recieveMessage(void) const;

	int32_t	closeSocket(void);

};

#endif /* ******************************************************************* */
