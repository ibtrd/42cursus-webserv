#ifndef SERVER_HPP
# define SERVER_HPP

# include "Configuration.hpp"
# include "Client.hpp"

# include <stdint.h>
# include <sys/epoll.h>
# include <map>

# define MAX_EVENTS 64

typedef std::map<fd_t, std::vector<ServerBlock> > servermap_t;
typedef std::map<fd_t, std::vector<struct sockaddr_in> > bindmap_t;

class Server {
public:
	Server();
	~Server();

	void	configure(const Configuration &config);
	void	routine(void);

	int32_t	epollFd(void) const;

private:
	int32_t 					_epollFd;
	servermap_t					_serverBlocks;
	struct epoll_event 			_events[MAX_EVENTS];
	std::map<int32_t, Client>	_clients;

	fd_t	_addSocket(const ServerBlock &block, const struct sockaddr_in &host);
	error_t	_addConnection(const int32_t socket);
	void	_removeConnection(const int32_t socket);
};

#endif
