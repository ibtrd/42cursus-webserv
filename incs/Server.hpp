#ifndef SERVER_HPP
# define SERVER_HPP

# include "Configuration.hpp"
# include "Client.hpp"

# include <stdint.h>
# include <sys/epoll.h>
# include <map>
# include <list>

# define MAX_EVENTS 64
# define MIME_TYPE_FILE "/etc/mime.types"

typedef std::map<fd_t, std::vector<ServerBlock> > servermap_t;
typedef std::map<fd_t, std::vector<struct sockaddr_in> > socketbindmap_t;
typedef std::map<fd_t, std::list<Client>::iterator> clientbindmap_t;

class Server {
public:
	Server();
	~Server();

	void	configure(const Configuration &config);
	void	routine(void);
	const std::string	&getMimeType(const std::string &ext) const;
	const ServerBlock	&findServerBlock(const fd_t socket, const std::string &host) const;

	int32_t	epollFd(void) const;

private:
	int32_t 							_epollFd;
	servermap_t							_serverBlocks;
	struct epoll_event 					_events[MAX_EVENTS];
	std::list<Client>					_clients;
	clientbindmap_t						_fdClientMap;
	std::map<std::string, std::string>	_mimetypes;

	fd_t	_addSocket(const ServerBlock &block, const struct sockaddr_in &host);
	error_t	_addConnection(const int32_t socket);
	void	_removeConnection(const int32_t socket);
	error_t	_loadMimeTypes(void);
};

#endif
