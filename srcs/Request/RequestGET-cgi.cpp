#include "RequestGET.hpp"

#include <unistd.h>

#include "Server.hpp"
#include "ft.hpp"
#include "CgiBuilder.hpp"

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

	// this->_context.response.setStatusCode(STATUS_OK);

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
		std::cerr << "RequestGET CGI: " << this->_cgiPath->string() << std::endl;
	}
}

error_t RequestGET::_executeCGI(void) {
	CgiBuilder builder(this);
	
	if (-1 == dup2(this->_context.cgiSockets[CHILD_SOCKET], STDOUT_FILENO)
		|| -1 == dup2(this->_context.cgiSockets[CHILD_SOCKET], STDIN_FILENO)) {
		std::exit(1);
	}
	// std::cerr << builder;

	char **envp = builder.envp();
	char **argv = builder.argv();

	// std::cerr << "CGI-argv:\n";
	// for (uint32_t i = 0; argv[i]; ++i) {
	// 	std::cerr << argv[i] << "\n";
	// }
	// std::cerr << "CGI-envp:\n";
	// for (uint32_t i = 0; envp[i]; ++i) {
	// 	std::cerr << envp[i] << "\n";
	// }
	// std::cerr.flush();
	
	close(this->_context.cgiSockets[PARENT_SOCKET]);
	// close(STDIN_FILENO);

	std::cerr << "EXECVE! [GET]" << std::endl;
	execve(this->_cgiPath->string().c_str(), argv, envp);
	// execlp("/bin/ls", "ls", NULL, NULL);

	std::cerr << "execve(): " << strerror(errno) << std::endl;
	CgiBuilder::destroy(argv);
	CgiBuilder::destroy(envp);
	std::exit(1);
}
