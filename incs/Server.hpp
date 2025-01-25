#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdint.h>
#include <sys/epoll.h>

#include <list>
#include <map>

#include "Client.hpp"
#include "Configuration.hpp"

#define MAX_EVENTS 64
#define EPOLL_WAIT_TIMEOUT 1000
#define REQUEST_TIMEOUT 2
#define DEFAULT_MIMETYPE "application/octet-stream"

typedef std::map<fd_t, std::vector<ServerBlock> >        servermap_t;
typedef std::map<fd_t, std::vector<struct sockaddr_in> > socketbindmap_t;
typedef std::map<fd_t, std::list<Client>::iterator>      clientbindmap_t;

class Server {
public:
	Server();
	~Server();

	void configure(const Configuration &config);
	void routine(void);

	const std::string &getMimeType(const std::string &ext) const;
	const ServerBlock &findServerBlock(const fd_t socket, const std::string &host) const;
	int32_t            getTimeout(const uint32_t type) const;
	// const int32_t		*getTimeouts(void) const;

	error_t           addCGIToClientMap(const fd_t socket, const Client &client);

	int32_t epollFd(void) const;

private:
	int32_t            _epollFd;
	servermap_t        _serverBlocks;
	struct epoll_event _events[MAX_EVENTS];
	std::list<Client>  _clients;
	clientbindmap_t    _fdClientMap;
	mimetypes_t        _mimetypes;
	int32_t            _timeouts[TIMEOUT_COUNT];

	fd_t    _addSocket(const ServerBlock &block, const struct sockaddr_in &host);
	error_t _addConnection(const int32_t socket);
	void    _removeConnection(const fd_t fd);
	void    _checkClientsTimeout(void);
};

#endif
