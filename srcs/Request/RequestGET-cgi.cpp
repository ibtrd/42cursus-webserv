#include <unistd.h>

#include "CgiBuilder.hpp"
#include "RequestGET.hpp"
#include "Server.hpp"
#include "ft.hpp"

void RequestGET::_openCGI(void) {
	if (0 != this->_cgiPath->access(X_OK)) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, this->_context.cgiSockets)) {
		std::cerr << "Error: socketpair(): " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	this->_context.option = EPOLLIN;

	this->_context.pid = fork();
	if (-1 == this->_context.pid) {
		std::cerr << "Error: fork(): " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}

	if (this->_context.pid == 0) {
		try {
			this->_executeCGI();
		} catch (...) {
			std::exit(1);
		}
	} else {
		close(this->_context.cgiSockets[CHILD_SOCKET]);
		shutdown(this->_context.cgiSockets[PARENT_SOCKET], SHUT_WR);
		SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		UNSET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
	}
}
