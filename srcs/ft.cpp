#include "ft.hpp"

namespace ft {
std::string numToStr(int num) {
	std::string       str;
	std::stringstream ss;
	ss << num;
	ss >> str;
	return (str);
}

std::string generateRandomString(size_t len) {
	std::string str;
	str.reserve(len);
	for (size_t i = 0; i < len; ++i) {
		str.push_back('a' + (std::rand() % 26));
	}
	return (str);
}
}  // namespace ft
