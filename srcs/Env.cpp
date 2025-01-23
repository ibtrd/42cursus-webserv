#include "Env.hpp"

#include <arpa/inet.h>

#include <algorithm>
#include <cstring>

/* CONSTRUCTORS ************************************************************* */

Env::Env(void) {}

Env::Env(const Env &other) { *this = other; }

Env::Env(const RequestContext_t &context) {
	this->_addContext(context);
	this->add("SERVER_SOFTWARE", WEBSERV_VERSION);
}

Env::~Env(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Env &Env::operator=(const Env &other) {
	if (this == &other) {
		return (*this);
	}
	this->_envars = other._envars;
	return (*this);
}

/* ************************************************************************** */

void Env::add(const std::string &key, const std::string &val) {
	this->_envars.push_back(key + "=" + val);
}

char **Env::envp(void) const {
	char **env                = new char *[this->_envars.size() + 1];
	env[this->_envars.size()] = NULL;
	for (uint32_t i = 0; i < this->_envars.size(); ++i) {
		env[i] = new char[this->_envars.at(i).size() + 1];
		if (NULL == env[i]) {
			while (i--) {
				delete[] env[i];
			}
			delete[] env;
			return NULL;
		}
		std::strcpy(env[i], this->_envars.at(i).c_str());
	}
	return env;
}

void Env::destroy(char **envp) {
	for (uint32_t i = 0; NULL != envp[i]; ++i) {
		delete[] envp[i];
	}
	delete[] envp;
}

/* ************************************************************************** */

void Env::_addContext(const RequestContext_t &context) {
	this->add("REQUEST_METHOD", context.method.string());
	this->_addHeaders(context.headers);
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &context.addr.sin_addr, clientIP, INET_ADDRSTRLEN);
	this->add("REMOTE_ADDR", clientIP);
	this->add("REMOTE_HOST", clientIP);
}

void Env::_addHeaders(const headers_t &headers) {
	for (headers_t::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::string var = it->first;
		std::replace(var.begin(), var.end(), '-', '_');
		for (std::string::iterator it = var.begin(); it != var.end(); ++it) {
			*it = toupper(*it);
		}
		this->add("HTTP_" + var, it->second);
	}
}

/* ************************************************************************** */

std::ostream &operator<<(std::ostream &os, const Env &env) {
	for (uint32_t i = 0; i < env._envars.size(); ++i) {
		os << env._envars.at(i) << "\n";
	}
	os.flush();
	return os;
}
