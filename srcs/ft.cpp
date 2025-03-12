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

int hexToDec(char c) {
	if (c >= '0' && c <= '9') {
		return (c - '0');
	}
	if (c >= 'A' && c <= 'F') {
		return (c - 'A' + 10);
	}
	if (c >= 'a' && c <= 'f') {
		return (c - 'a' + 10);
	}
	return (0);
}

std::string decodeURI(std::string uri) {
	std::string decoded;
	decoded.reserve(uri.size());
	for (size_t i = 0; i < uri.size(); ++i) {
		if (uri[i] == '%' && i + 2 < uri.size()) {
			decoded.push_back((hexToDec(uri[i + 1]) << 4) + hexToDec(uri[i + 2]));
			i += 2;
		} else {
			decoded.push_back(uri[i]);
		}
	}
	return (decoded);
}
}  // namespace ft
