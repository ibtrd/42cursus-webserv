#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <arpa/inet.h>

class Client
{
	public:
		Client();
		Client(int socket_fd, struct sockaddr_in addr);
		Client(const Client &src);
		~Client();

		Client &operator=(const Client &src);

		int getSocketFd() const { return socket_fd; }
		struct sockaddr_in getAddr() const { return addr; }
		socklen_t getAddrlen() const { return addrlen; }

		// void send(const char *msg);
		// void recv(char *msg, size_t size);
		void closeSocket(void);

	private:
		int					socket_fd;
		struct sockaddr_in	addr;
		socklen_t			addrlen;
};

#endif
