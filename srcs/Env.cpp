#include "Env.hpp"

#include <stdint.h>

#include <cstring>
#include <ostream>

/* CONSTRUCTORS ************************************************************* */

Env::Env(void) {}

Env::Env(const Env &other) { *this = other; }

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

/* ************************************************************************** */

std::ostream &operator<<(std::ostream &os, const Env &env) {
	for (uint32_t i = 0; i < env._envars.size(); ++i) {
		os << env._envars.at(i) << "\n";
	}
	os.flush();
	return os;
}
