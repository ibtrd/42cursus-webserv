#ifndef SERVER_HPP
# define SERVER_HPP

# include "Configuration.hpp"
# include "RequestMaster.hpp"
# include "RequestGET.hpp"

# include <stdint.h>
# include <sys/epoll.h>
# include <map>

# define MAX_EVENTS 64

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
	std::map<int32_t, ARequest *>	_requests;

	fd_t	_addSocket(const ServerBlock &block, const struct sockaddr_in &host);
	error_t	_addConnection(const int32_t socket);
	void	_removeConnection(const int32_t socket);
	error_t	_transformRequest(const int32_t socket);
};

#endif
