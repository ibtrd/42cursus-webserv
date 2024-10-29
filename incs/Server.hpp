#ifndef SERVER_HPP
# define SERVER_HPP

# include "Configuration.hpp"
# include "Request.hpp"

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
	int32_t 					_serverSocket;
	int32_t 					_epollFd;
	struct epoll_event 			_events[MAX_EVENTS];
	std::map<int32_t, Request>	_requests;

	void	_init(void);
	void	_addConnection(const int32_t socket);
};

#endif
