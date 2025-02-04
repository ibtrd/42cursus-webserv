#ifndef PATH_HPP
#define PATH_HPP

#include <sys/stat.h>

#include <ctime>
#include <vector>

#include "webdef.hpp"

class Path {
public:
	Path(void);
	Path(const Path &other);
	Path(const std::string &str);

	~Path(void);

	Path &operator=(const Path &other);
	bool  operator==(const Path &other);
	bool  operator!=(const Path &other);

	bool    empty(void) const;
	bool    isOriginForm(void) const;
	bool    isFileFormat(void) const;
	bool    isDirFormat(void) const;
	int     access(int type) const;
	error_t stat(void);

	bool          isFile(void) const;
	bool          isDir(void) const;
	const time_t &mTime(void) const;
	long          size(void) const;
	dev_t         deviceID(void) const;

	const std::string &string(void) const;
	const char        *c_str(void) const;

	std::string extension(void) const;
	Path        dir(void) const;
	std::string notdir(void) const;
	uint32_t    length(void) const;
	uint32_t    prefixLength(void) const;
	bool        prefixMatch(const Path &other) const;
	std::string concat(const Path &other) const;

private:
	std::string              _str;
	std::vector<std::string> _chunks;
	struct stat              _stat;
	bool                     _statDone;

	friend std::ostream &operator<<(std::ostream &os, const Path &path);
};

#endif /* ******************************************************************* */
