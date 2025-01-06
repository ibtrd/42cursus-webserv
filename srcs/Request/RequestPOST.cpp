#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

#include "RequestPOST.hpp"
#include "ft.hpp"

/* CONSTRUCTORS ************************************************************* */

// RequestPOST::RequestPOST(void)
// {
// 	// std::cerr << "RequestPOST created" << std::endl;
// }

RequestPOST::RequestPOST(Client &client) : ARequest(client)
{
	std::cerr << "RequestPOST created" << std::endl;
}

RequestPOST::RequestPOST(const RequestPOST &other) : ARequest(other)
{
	// std::cerr << "RequestPOST copy" << std::endl;
	*this = other;
}

RequestPOST::~RequestPOST(void)
{
	// std::cerr << "RequestPOST destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestPOST	&RequestPOST::operator=(const RequestPOST &other)
{
	std::cerr << "RequestPOST assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

error_t	RequestPOST::process(void)
{
	std::cerr << "RequestPOST process" << std::endl;
	return (REQ_DONE);
}

ARequest	*RequestPOST::clone(void) const
{
	std::cerr << "RequestPOST clone" << std::endl;
	return (new RequestPOST(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest	*createRequestPOST(Client &client)
{
	std::cerr << "createRequestPOST" << std::endl;
	return (new RequestPOST(client));
}
