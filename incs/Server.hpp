#ifndef SERVER_HPP
# define SERVER_HPP

# include "Client.hpp"
# include <arpa/inet.h>
# include <vector>

class Server
{
	public:
		Server(/* args */);
		Server(int port);
		Server(const Server &src);
		~Server();
		Server &operator=(const Server &src);

		int getServerSocket() const { return serverSocket; }

		void acceptClient(void);
		void dialog(void);
		void closeClient(void);

	private:
		int serverSocket;
		// int opt;
		struct sockaddr_in addr;
		socklen_t addrlen;
		// std::vector<Client> clients;
		Client client;	// 1 client at a time for now
};

#endif
