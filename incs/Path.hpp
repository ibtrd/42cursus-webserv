#ifndef PATH_HPP
# define PATH_HPP

#include <string>
#include <vector>
#include <stdint.h>

#define DIR_MATCH -1
#define FILE_MATCH -2

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
	int32_t		match(const Path &other) const;

private:
	std::string					_str;
	std::vector<std::string>	_chunks;
};

#endif /* ******************************************************************* */
