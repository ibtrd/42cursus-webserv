
#include <unistd.h>

#include "CgiBuilder.hpp"
#include "ft.hpp"

/* STATIC VARIABLES ********************************************************* */

const char *ARequest::_chunkTerminator[CHUNK_TERMINATOR_SIZE] = {"0\r\n\r\n", "0\r\n\r", "0\r\n",
                                                                 "0\r", "0"};

/* CONSTRUCTORS ************************************************************* */

ARequest::ARequest(RequestContext_t &context)
    : _context(context), _chunked(false), _contentLength(0), _cgiSilent(true) {
	uint32_t    matchLength  = this->_context.ruleBlock->path().string().size() - 1;
	std::string chopedTarget = this->_context.target.substr(matchLength, std::string::npos);
	const Path &root         = this->_context.ruleBlock->getRoot().string();

	if (!chopedTarget.empty()) {
		this->_path = root.concat(chopedTarget);
	} else {
		this->_path = root.string().substr(0, root.string().size() - 1);
	}

	this->_path = ft::decodeURI(this->_path.string());

	this->_cgiPath = this->_context.ruleBlock->findCGI(this->_path.extension());
}

ARequest::ARequest(const ARequest &other)
    : _context(other._context),
      _chunked(other._chunked),
      _contentLength(other._contentLength),
      _cgiSilent(other._cgiSilent) {
	uint32_t    matchLength  = this->_context.ruleBlock->path().string().size() - 1;
	std::string chopedTarget = this->_context.target.substr(matchLength, std::string::npos);
	const Path &root         = this->_context.ruleBlock->getRoot().string();

	if (!chopedTarget.empty()) {
		this->_path = root.concat(chopedTarget);
	} else {
		this->_path = root.string().substr(0, root.string().size() - 1);
	}

	this->_cgiPath = this->_context.ruleBlock->findCGI(this->_path.extension());
}

ARequest::~ARequest(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

ARequest &ARequest::operator=(const ARequest &other) {
	if (this == &other) {
		return (*this);
	}
	return (*this);
}

/* PRIVATE METHODS ********************************************************** */

error_t ARequest::_executeCGI(void) {
	if (-1 == dup2(this->_context.cgiSockets[CHILD_SOCKET], STDOUT_FILENO) ||
	    -1 == dup2(this->_context.cgiSockets[CHILD_SOCKET], STDIN_FILENO)) {
		std::exit(1);
	}
	close(this->_context.cgiSockets[PARENT_SOCKET]);
	close(this->_context.cgiSockets[CHILD_SOCKET]);

	CgiBuilder builder(this);

	if (0 != CgiBuilder::chdir(this)) {
		std::cerr << "Error: chdir(): " << strerror(errno) << std::endl;
		std::exit(1);
	}
	char **envp = builder.envp();
	char **argv = builder.argv();
	execve(this->_cgiPath->string().c_str(), argv, envp);
	std::cerr << "Error: execve(): " << strerror(errno) << std::endl;
	CgiBuilder::destroy(envp);
	CgiBuilder::destroy(argv);
	std::exit(1);
}

/* ************************************************************************** */

error_t ARequest::workIn(void) {
	throw std::logic_error("ARequest::workIn should not be called");
	return (REQ_DONE);
}

error_t ARequest::workOut(void) {
	throw std::logic_error("ARequest::workOut should not be called");
	return (REQ_ERROR);
}

error_t ARequest::CGIIn(void) {
	throw std::logic_error("ARequest::CGIIn should not be called");
	return (REQ_ERROR);
}

error_t ARequest::CGIOut(void) {
	throw std::logic_error("ARequest::CGIOut should not be called");
	return (REQ_ERROR);
}

/* GETTERS ****************************************************************** */

const RequestContext_t &ARequest::context(void) const { return this->_context; }

const Path &ARequest::path(void) const { return this->_path; }
const Path &ARequest::cgiPath(void) const { return *this->_cgiPath; }

bool ARequest::CGISilent(void) const { return this->_cgiSilent; }

/* ************************************************************************** */
