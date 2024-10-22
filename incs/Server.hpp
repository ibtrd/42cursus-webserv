#ifndef SERVER_HPP
# define SERVER_HPP

# include "Client.hpp"
# include <arpa/inet.h>
# include <vector>

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
		fd_set readfds;
		std::vector<Client> clients;
		// Client client;	// 1 client at a time for now
};

#endif
