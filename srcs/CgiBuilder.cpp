#include "CgiBuilder.hpp"
#include "ft.hpp"

#include <arpa/inet.h>

#include <algorithm>
#include <cstring>

/* CONSTRUCTORS ************************************************************* */

CgiBuilder::CgiBuilder(void) {}

CgiBuilder::CgiBuilder(const CgiBuilder &other) { *this = other; }

CgiBuilder::CgiBuilder(const ARequest *req) {
	this->_addContext(req->context());
    this->addEnvar("SCRIPT_FILENAME", req->path().string());
	this->addEnvar("GATEWAY_INTERFACE", CGI_PROTOCOL_VERSION);
	this->addEnvar("SERVER_PROTOCOL", HTTP_PROTOCOL_VERSION);
	this->addEnvar("SERVER_SOFTWARE", WEBSERV_VERSION);
	this->addArgument(req->cgiPath().string());
	this->addArgument(req->path().string());
}

CgiBuilder::~CgiBuilder(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

CgiBuilder &CgiBuilder::operator=(const CgiBuilder &other) {
	if (this == &other) {
		return (*this);
	}
	this->_envars = other._envars;
	this->_arguments = other._arguments;
	return (*this);
}

/* ************************************************************************** */

void CgiBuilder::addEnvar(const std::string &key, const std::string &val) {
	this->_envars.push_back(key + "=" + val);
}

void CgiBuilder::addArgument(const std::string &arg) {
	this->_arguments.push_back(arg);
}

char **CgiBuilder::envp(void) const {
	char **envp = new char *[this->_envars.size() + 1];
	envp[this->_envars.size()] = NULL;
	for (uint32_t i = 0; i < this->_envars.size(); ++i) {
		envp[i] = new char[this->_envars.at(i).size() + 1];
		if (NULL == envp[i]) {
			while (i--) {
				delete[] envp[i];
			}
			delete[] envp;
			return NULL;
		}
		std::strcpy(envp[i], this->_envars.at(i).c_str());
	}
	return envp;
}

char **CgiBuilder::argv(void) const {
	char **argv = new char *[this->_arguments.size() + 1];
	argv[this->_arguments.size()] = NULL;
	for (uint32_t i = 0; i < this->_arguments.size(); ++i) {
		argv[i] = new char[this->_arguments.at(i).size() + 1];
		if (NULL == argv[i]) {
			while (i--) {
				delete[] argv[i];
			}
			delete[] argv;
			return NULL;
		}
		std::strcpy(argv[i], this->_arguments.at(i).c_str());
	}
	return argv;
}

void CgiBuilder::destroy(char **envp) {
	for (uint32_t i = 0; NULL != envp[i]; ++i) {
		delete[] envp[i];
	}
	delete[] envp;
}

/* ************************************************************************** */

void CgiBuilder::_addContext(const RequestContext_t &context) {
	this->addEnvar("REQUEST_METHOD", context.method.string());
	this->_addHeaders(context.headers);
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &context.addr.sin_addr, clientIP, INET_ADDRSTRLEN);
	this->addEnvar("REMOTE_ADDR", clientIP);
	this->addEnvar("REMOTE_HOST", clientIP);
	this->addEnvar("QUERY_STRING", context.queries.queryLine());
	this->addEnvar("REDIRECT_STATUS", "");
	this->addEnvar("PATH_INFO", context.target);
	this->addEnvar("REQUEST_URI", context.target + context.queries.originalQueryLine());
}

void CgiBuilder::_addHeaders(const headers_t &headers) {
	for (headers_t::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::string var = it->first;
		std::replace(var.begin(), var.end(), '-', '_');
		for (std::string::iterator it = var.begin(); it != var.end(); ++it) {
			*it = toupper(*it);
		}
		this->addEnvar("HTTP_" + var, it->second);
	}
}

/* ************************************************************************** */

std::ostream &operator<<(std::ostream &os, const CgiBuilder &envp) {
	os << "CGI-envp:\n";
	for (uint32_t i = 0; i < envp._envars.size(); ++i) {
		os << envp._envars.at(i) << "\n";
	}
	os << "CGI-argv:\n";
	for (uint32_t i = 0; i < envp._arguments.size(); ++i) {
		os << envp._arguments.at(i) << "\n";
	}
	os.flush();
	return os;
}
