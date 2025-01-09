#ifndef PATH_HPP
# define PATH_HPP

#include <string>
#include <vector>
#include <stdint.h>

class Path {
public:
	Path(void);
	Path(const Path &other);
	Path(const std::string &str);

	~Path(void);

	Path	&operator=(const Path &other);

	const std::string	&string(void) const;
	std::string			extension(void) const;

	bool	isOriginForm(void) const;
	bool	isFile(void) const;
	bool	isDir(void) const;

	uint32_t	length(void) const;
	uint32_t	prefixLength(void) const;
	bool		prefixMatch(const Path &other) const;

private:
	std::string					_str;
	std::vector<std::string>	_chunks;

	friend std::ostream &operator<<(std::ostream &os, const Path &path);
};

#endif /* ******************************************************************* */
