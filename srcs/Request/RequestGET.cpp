#include "RequestGET.hpp"

#include <unistd.h>


#include "Server.hpp"
#include "ft.hpp"
#include "webservHTML.hpp"
#include "CgiBuilder.hpp"

/* CONSTRUCTORS ************************************************************* */

RequestGET::RequestGET(RequestContext_t &context) : ARequest(context), _dir(NULL) {
	// std::cerr << "RequestGET created" << std::endl;
	SET_REQ_WORK_IN_COMPLETE(this->_context.requestState);  // No workIn needed
	SET_REQ_CGI_OUT_COMPLETE(this->_context.requestState);
		SET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
}

RequestGET::RequestGET(const RequestGET &other) : ARequest(other), _dir(NULL) {
	// std::cerr << "RequestGET copy" << std::endl;
	*this = other;
}

RequestGET::~RequestGET(void) {
	// std::cerr << "RequestGET destroyed" << std::endl;
	if (this->_file.is_open()) {
		this->_file.close();
	}
	if (this->_dir != NULL) {
		closedir(this->_dir);
	}
}

/* OPERATOR OVERLOADS ******************************************************* */

RequestGET &RequestGET::operator=(const RequestGET &other) {
	// std::cerr << "RequestGET assign" << std::endl;
	(void)other;
	return (*this);
}

/* ************************************************************************** */

void RequestGET::_openFile(void) {
	// std::cerr << "RequestGET _openFile" << std::endl;
	this->_file.open(this->_path.c_str(), std::ios::in | std::ios::binary);
	if (!this->_file.is_open()) {
		std::cerr << "open(): " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setHeader(HEADER_CONTENT_TYPE,
	                                  this->_context.server.getMimeType(this->_path.extension()));
	this->_context.response.setHeader(HEADER_CONTENT_LENGTH, ft::numToStr(this->_path.size()));
}

void RequestGET::_openDir(void) {
	// std::cerr << "RequestGET _openDir" << std::endl;
	this->_dir = opendir(this->_path.string().c_str());
	if (this->_dir == NULL) {
		std::cerr << "Error: opendir(): " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}
	this->_context.response.setStatusCode(STATUS_OK);
	this->_context.response.setHeader(HEADER_CONTENT_TYPE, "text/html");
	this->_context.response.setBody(INDEXOF(this->_context.target));
}

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

	this->_context.response.setStatusCode(STATUS_OK);

	this->_context.pid = fork();
	if (-1 == this->_context.pid) {
		std::cerr << "Error: fork(): " << strerror(errno) << std::endl;
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return;
	}

	if (this->_context.pid == 0) {
		this->_executeCGI();
	} else {
		close(this->_context.cgiSockets[CHILD_SOCKET]);
		SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		UNSET_REQ_CGI_IN_COMPLETE(this->_context.requestState);
		std::cerr << "RequestGET CGI: " << this->_cgiPath->string() << std::endl;
	}
}

error_t RequestGET::_readFile(void) {
	uint8_t buffer[REQ_BUFFER_SIZE];

	this->_file.read((char *)buffer, REQ_BUFFER_SIZE);
	ssize_t bytes = this->_file.gcount();
	if (bytes == 0) {
		SET_REQ_WORK_OUT_COMPLETE(this->_context.requestState);
		return (REQ_CONTINUE);
	}

	this->_context.responseBuffer.append(buffer, bytes);
	return (REQ_CONTINUE);
}

error_t RequestGET::_executeCGI(void) {
	CgiBuilder builder(this);
	
	// std::cerr << builder;

	char **envp = builder.envp();
	char **argv = builder.argv();

	std::cerr << "CGI-argv:\n";
	for (uint32_t i = 0; argv[i]; ++i) {
		std::cerr << argv[i] << "\n";
	}
	std::cerr << "CGI-envp:\n";
	for (uint32_t i = 0; envp[i]; ++i) {
		std::cerr << envp[i] << "\n";
	}
	std::cerr.flush();

	dup2(this->_context.cgiSockets[CHILD_SOCKET], STDOUT_FILENO);
	close(this->_context.cgiSockets[PARENT_SOCKET]);
	close(STDIN_FILENO);

	std::cerr << "EXECVE!" << std::endl;
	execve(this->_cgiPath->string().c_str(), argv, envp);
	// execlp("/bin/ls", "ls", NULL, NULL);

	std::cerr << "execve(): " << strerror(errno) << std::endl;
	CgiBuilder::destroy(argv);
	CgiBuilder::destroy(envp);
	exit(1);
}

error_t RequestGET::_fetchIndexes(void) {
	for (std::vector<std::string>::const_iterator it = this->_context.ruleBlock->indexes().begin();
	     it != this->_context.ruleBlock->indexes().end(); ++it) {
		std::string test = this->_path.concat(*it);
		if (0 == access(test.c_str(), F_OK)) {
			this->_path = test;
			return 0;
		}
	}
	return -1;
}

error_t RequestGET::_validateLocalFile(void) {
	if (0 != this->_path.stat()) {
		this->_context.response.setStatusCode(STATUS_INTERNAL_SERVER_ERROR);
		return (REQ_DONE);
	}
	if (0 != this->_path.access(R_OK)) {
		this->_context.response.setStatusCode(STATUS_FORBIDDEN);
		return (REQ_DONE);
	}
	if (this->_path.isFile()) {
		if (this->_cgiPath) {
			this->_openCGI();
			std::cerr << "RequestGET CGIIIIIIIIIIII GET" << std::endl;
		} else {
			std::cerr << "RequestGET _validateLocalFile" << std::endl;
			this->_openFile();
		}
		return (REQ_DONE);
	}
	return (REQ_CONTINUE);
}

/* ************************************************************************** */

void RequestGET::processing(void) {
	// std::cerr << "RequestGET parse" << std::endl;
	if (0 != this->_path.access(F_OK)) {
		this->_context.response.setStatusCode(STATUS_NOT_FOUND);
		return;
	}
	if (REQ_CONTINUE != this->_validateLocalFile()) {
		return;
	}
	if (!this->_path.isDir()) {
		this->_context.response.setStatusCode(STATUS_CONFLICT);
		return;
	}
	// std::cerr << "RequestGET parse " << this->_path.isDirFormat() << std::endl;
	if (!this->_path.isDirFormat()) {
		this->_context.response.setStatusCode(STATUS_MOVED_PERMANENTLY);
		this->_context.response.setHeader(HEADER_LOCATION, this->_context.target + '/' + this->_context.queries.originalQueryLine());
		return;
	}
	if (0 == this->_fetchIndexes() &&
		REQ_CONTINUE != this->_validateLocalFile()) {
		return;
	} else if (this->_context.ruleBlock->isDirListing()){
		this->_openDir();
		return;
	}
	// this->_context.response.setStatusCode(STATUS_FORBIDDEN); //OG
	this->_context.response.setStatusCode(STATUS_NOT_FOUND);  // TESTER
}

error_t RequestGET::workOut(void) {
	// std::cerr << "RequestGET workOut" << std::endl;

	if (this->_path.isFileFormat()) {
		return (this->_readFile());
	}

	if (this->_path.isDirFormat()) {
		return (this->_readDir());
	}

	return (REQ_ERROR);
}

error_t RequestGET::CGIIn(void) {
	return (this->_readCGI());
}

ARequest *RequestGET::clone(void) const {
	// std::cerr << "RequestGET clone" << std::endl;
	return (new RequestGET(*this));
}

/* GETTERS ****************************************************************** */

/* SETTERS ****************************************************************** */

/* EXCEPTIONS *************************************************************** */

/* OTHERS *********************************************************************/

ARequest *createRequestGET(RequestContext_t &context) {
	// std::cerr << "createRequestGET" << std::endl;
	return (new RequestGET(context));
}
