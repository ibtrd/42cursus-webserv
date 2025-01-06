#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

#include "RequestGET.hpp"
#include "ft.hpp"
#include "Client.hpp"

/* CONSTRUCTORS ************************************************************* */

// RequestGET::RequestGET(void)
// {
// 	// std::cerr << "RequestGET created" << std::endl;
// }

RequestGET::RequestGET(Client &client) : ARequest(client)
{
	std::cerr << "RequestGET created" << std::endl;
}

RequestGET::RequestGET(const RequestGET &other) : ARequest(other)
{
	// std::cerr << "RequestGET copy" << std::endl;
	*this = other;
}

RequestGET::~RequestGET(void)
{
	// std::cerr << "RequestGET destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestGET	&RequestGET::operator=(const RequestGET &other)
{
	std::cerr << "RequestGET assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

error_t	RequestGET::process(void)
{
	std::cerr << "RequestGET process" << std::endl;
	return (REQ_DONE);
}

ARequest	*RequestGET::clone(void) const
{
	std::cerr << "RequestGET clone" << std::endl;
	return (new RequestGET(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest	*createRequestGET(Client &client)
{
	(void)client;
	std::cerr << "createRequestGET" << std::endl;
	return (new RequestGET(client));
}
