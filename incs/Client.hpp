#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <arpa/inet.h>

typedef struct	// will bechange to class
{
	struct sockaddr_in	addr;
	socklen_t			addrlen;
}	t_client;

#endif
