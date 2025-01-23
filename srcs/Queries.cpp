#include "Queries.hpp"

static bool isValideString(const std::string &str);

/* CONSTRUCTORS ************************************************************* */

Queries::Queries(void) : _queryLine(), _queries() {}

Queries::Queries(const Queries &other) : _queryLine(other._queryLine), _queries(other._queries) {}

Queries::Queries(const std::string &queryLine) : _queryLine(queryLine), _queries() { this->parse(); }

Queries::~Queries(void) {}

/* OPERATOR OVERLOADS ******************************************************* */

Queries &Queries::operator=(const Queries &other) {
	if (this == &other) {
		return (*this);
	}
	this->_queryLine = other._queryLine;
	this->_queries   = other._queries;
	return (*this);
}

/* ************************************************************************** */

void Queries::_parseQuery(const std::string &query) {
	size_t pos = query.find('=');
	if (pos == std::string::npos) {
		this->_queries[query] = "";
	} else {
		this->_queries[query.substr(0, pos)] = query.substr(pos + 1);
	}
}

/* ************************************************************************** */

void Queries::parse(void) {
	size_t pos = 0;
	size_t end = 0;

	while (pos < this->_queryLine.size()) {
		end = this->_queryLine.find(QUERY_DELIM, pos);
		if (end == std::string::npos) {
			end = this->_queryLine.size();
		}
		this->_parseQuery(this->_queryLine.substr(pos, end - pos));
		pos = end + 1;
	}
}

const std::string &Queries::queryLine(void) const { return (this->_queryLine); }

const std::string Queries::originalQueryLine(void) const {
	if (this->_queryLine.empty()) {
		return ("");
	}
	return ("?" + this->_queryLine);
}

const queries_t &Queries::queries(void) const { return (this->_queries); }

bool Queries::isValid(void) const {
	if (this->_queryLine.empty()) {
		return (false);
	}
	for (queries_t::const_iterator it = this->_queries.begin(); it != this->_queries.end(); ++it) {
		if (it->first.empty() || !isValideString(it->first) || !isValideString(it->second)) {
			return (false);
		}
	}
	return (true);
}

/* ************************************************************************** */

static bool isValideString(const std::string &str) {
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
		if (!isalnum(*it) && *it != '.' && *it != '-' && *it != '_' && *it != '~') {
			return (false);
		}
	}
	return (true);
}
