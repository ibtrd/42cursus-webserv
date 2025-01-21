#include "ft.hpp"

#include <sys/stat.h>

#include <string>

namespace ft {
std::string numToStr(int num) {
	std::string       str;
	std::stringstream ss;
	ss << num;
	ss >> str;
	return (str);
}

bool is_dir(char *path) {
	struct stat buf;

	if (stat(path, &buf))  // Error handling
	{
		return (false);
	}
	if (S_ISDIR(buf.st_mode)) {
		return (true);
	}
	return (false);
}

std::string generateRandomString(size_t len) {
	std::string str;
	str.reserve(len);
	for (size_t i = 0; i < len; ++i) {
		str.push_back('a' + (rand() % 26));
	}
	return (str);
}
}  // namespace ft
