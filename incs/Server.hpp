#ifndef SERVER_HPP
# define SERVER_HPP

# include "Client.hpp"
# include <arpa/inet.h>
# include <vector>
# include <sys/epoll.h>

# define MAX_EVENTS 10

class Server
{
	public:
		Server(/* args */);
		Server(int port, int queueSize = 1);
		Server(const Server &src);
		~Server();
		Server &operator=(const Server &src);

		int getServerSocket() const { return serverSocket; }

		void run(void);

		void sendAll(const char *msg);

	private:
		int serverSocket;
		// int opt;
		// int queueSize;
		struct sockaddr_in addr;
		socklen_t addrlen;
		struct epoll_event events[MAX_EVENTS];
		int epoll_fd;
		std::vector<int> clients;
};

#endif
