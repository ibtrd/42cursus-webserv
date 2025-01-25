#include "RequestPOST.hpp"

#include <unistd.h>


#include "Server.hpp"
#include "ft.hpp"
#include "webservHTML.hpp"
#include "CgiBuilder.hpp"

/* CONSTRUCTORS ************************************************************* */

RequestPOST::RequestPOST(RequestContext_t &context) : ARequest(context) {
	// std::cerr << "RequestPOST created" << std::endl;
	SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);  // No workIn needed
	SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
		SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
}

RequestPOST::RequestPOST(const RequestPOST &other) : ARequest(other) {
	// std::cerr << "RequestPOST copy" << std::endl;
	*this = other;
}

RequestPOST::~RequestPOST(void) {
	// std::cerr << "RequestPOST destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestPOST &RequestPOST::operator=(const RequestPOST &other) {
	// std::cerr << "RequestPOST assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

void RequestPOST::_openCGI(void) {
	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, this->_context._cgiSockets)) {
		std::cerr << "Error: socketpair: " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}

	this->_context.response.setStatusCode(STATUS_OK);

	this->_context._pid = fork();
	if (-1 == this->_context._pid) {
		std::cerr << "Error: fork: " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}

	if (this->_context._pid == 0) {
		this->_executeCGI();
	} else {
		close(this->_context._cgiSockets[CHILD_SOCKET]);
		SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		UNSET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
		this->_context.response.enableIsCgi();
		std::cerr << "RequestPOST CGI: " << this->_cgiPath->string() << std::endl;
	}
}

error_t RequestPOST::_readCGI(void) {
	uint8_t buffer[REQ_BUFFER_SIZE];

	ssize_t bytes = read(this->_context._cgiSockets[PARENT_SOCKET], buffer, REQ_BUFFER_SIZE);
	if (bytes == 0) {
		std::cerr << "read: EOF" << std::endl;
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
		return (REQ_CONTINUE);
	}
	if (bytes == -1) {
		std::cerr << "read: " << strerror(errno) << std::endl;
		// this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		SET_REQ_WORK_COMPLETE(this->_context.requestState);
		return (REQ_ERROR);
	}

	this->_context.responseBuffer.append(buffer, bytes);
	return (REQ_CONTINUE);
}

error_t RequestPOST::_executeCGI(void) {
	CgiBuilder builder(this);

	char **envp = builder.envp();
	char **argv = builder.argv();

	// std::cerr << "ARGV: " << std::endl;
	// std::cerr << argv[0] << std::endl;
	// std::cerr << argv[1] << std::endl;
	// // std::cerr << argv[2] << std::endl;
	// std::cerr << "----------------" << std::endl;
	// std::cerr << "ENV: " << builder << std::endl;

	dup2(this->_context._cgiSockets[CHILD_SOCKET], STDOUT_FILENO);
	close(this->_context._cgiSockets[PARENT_SOCKET]);

	execve(this->_cgiPath->string().c_str(), argv, envp);
	// execlp("/bin/ls", "ls", NULL, NULL);

	std::cerr << "execlp: " << strerror(errno) << std::endl;

	CgiBuilder::destroy(envp);
	CgiBuilder::destroy(argv);

	exit(1);
}

error_t RequestPOST::_validateLocalFile(void) {
	if (0 != this->_path.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return (REQ_DONE);
	}
	if (0 != this->_path.access(R_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		return (REQ_DONE);
	}
	if (this->_path.isFile()) {
		this->_openCGI();
		std::cerr << "RequestPOST CGIIIIIIIIIIII" << std::endl;
		return (REQ_DONE);
	}
	return (REQ_CONTINUE);
}

/* ************************************************************************** */

void RequestPOST::processing(void) {
	std::cerr << "RequestPOST parse" << std::endl;
	if (!this->_cgiPath) {
		this->_context.response.setStatusCode(STATUS_I_AM_A_TEAPOT);
		return;
	}
	if (0 != this->_path.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
		return;
	}
	if (REQ_CONTINUE != this->_validateLocalFile()) {
		return;
	}
	// this->_context.response.setStatusCode(STATUS_FORBIDDEN); //OG
	this->_context.response.setStatusCode(STATUS_I_AM_A_TEAPOT);  // TESTER
}

error_t RequestPOST::CGIIn(void) {
	return (this->_readCGI());
}

ARequest *RequestPOST::clone(void) const {
	// std::cerr << "RequestPOST clone" << std::endl;
	return (new RequestPOST(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest *createRequestPOST(RequestContext_t &context) {
	// std::cerr << "createRequestPOST" << std::endl;
	return (new RequestPOST(context));
}
