#ifndef QUERIES_HPP
# define QUERIES_HPP

# include <map>
# include <string>

# define QUERY_DELIM '&'

typedef std::map<std::string, std::string> queries_t;

class Queries {
private:
	std::string _queryLine;
	queries_t   _queries;

	void _parseQuery(const std::string &query);

public:
	Queries(void);
	Queries(const Queries &other);
	Queries(const std::string &queryLine);

	~Queries(void);

	Queries &operator=(const Queries &other);

	void parse(void);

	const std::string &queryLine(void) const;
	const std::string originalQueryLine(void) const;
	const queries_t   &queries(void) const;
	bool              isValid(void) const;
};


#endif
