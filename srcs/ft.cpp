#include <sys/stat.h>

#include "ft.hpp"

namespace ft {
	std::string	numToStr(int num)
	{
		std::string str;
		std::stringstream ss;
		ss << num;
		ss >> str;
		return (str);
	}

	bool	is_dir(char *path)
	{
		struct stat	buf;

		if (stat(path, &buf))	// Error handling
		{
			return (false);
		}
		if (S_ISDIR(buf.st_mode))
			return (true);
		return (false);
	}
}
