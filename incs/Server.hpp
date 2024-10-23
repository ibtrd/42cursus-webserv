#ifndef SERVER_HPP
# define SERVER_HPP

# include "Configuration.hpp"
# include "Client.hpp"

# include <stdint.h>
# include <sys/epoll.h>
# include <map>

# define MAX_EVENTS 64

class Server
{
	public:
		Server();
		Server(const Configuration &config);	// tmp
		Server(const Server &src);

		~Server();

		Server &operator=(const Server &src);

		void routine(void);
	private:
		int32_t _serverSocket;
		int32_t _epollFd;
		// struct epoll_event _event[MAX_EVENTS]; // ne passe pas la compilation car pas utilisé
		std::map<int32_t, t_client> _clients;

		void _init(void);
};

#endif
