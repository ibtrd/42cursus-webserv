#include "Client.hpp"

/* STATIC VARIABLES ********************************************************* */

const char *ARequest::_chunkTerminator[CHUNK_TERMINATOR_SIZE] = {"0\r\n\r\n", "0\r\n\r", "0\r\n",
                                                                   "0\r", "0"};

/* CONSTRUCTORS ************************************************************* */

// ARequest::ARequest(void)
// {
// 	// std::cerr << "ARequest created" << std::endl;
// }

ARequest::ARequest(RequestContext_t &context) : _context(context) {
	// std::cerr << "ARequest created" << std::endl;
	uint32_t matchLength = this->_context.ruleBlock->path().string().size() - 1;
	std::string chopedTarget = this->_context.target.substr(matchLength, std::string::npos);
	const Path &root = this->_context.ruleBlock->getRoot().string();
	
	if (!chopedTarget.empty()) {
		this->_path = root.concat(chopedTarget);
	} else {
		this->_path = root.string().substr(0, root.string().size() - 1);
	}

	this->_cgiPath = this->_context.ruleBlock->findCGI(this->_path.extension());
}

ARequest::ARequest(const ARequest &other) : _context(other._context) {
	// std::cerr << "ARequest copy" << std::endl;
	uint32_t matchLength = this->_context.ruleBlock->path().string().size() - 1;
	std::string chopedTarget = this->_context.target.substr(matchLength, std::string::npos);
	const Path &root = this->_context.ruleBlock->getRoot().string();
	
	if (!chopedTarget.empty()) {
		this->_path = root.concat(chopedTarget);
	} else {
		this->_path = root.string().substr(0, root.string().size() - 1);
	}

	this->_cgiPath = this->_context.ruleBlock->findCGI(this->_path.extension());
}

ARequest::~ARequest(void) {
	// std::cerr << "ARequest destroyed" << std::endl;
}

/* OPERATOR OVERLOADS ******************************************************* */

ARequest &ARequest::operator=(const ARequest &other) {
	std::cerr << "ARequest assign" << std::endl;
	if (this == &other) {
		return (*this);
	}
	// this->_context = other._context;
	return (*this);
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

/* GETTERS ****************************************************************** */

const RequestContext_t &ARequest::context(void) const {
	return this->_context;
}

const Path &ARequest::path(void) const {
	return this->_path;
}
const Path &ARequest::cgiPath(void) const {
	return *this->_cgiPath;
}

/* ************************************************************************** */
