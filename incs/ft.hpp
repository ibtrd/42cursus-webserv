#ifndef FT_HPP
#define FT_HPP

#include <errno.h>

#include <cstdlib>
#include <cstring>
#include <limits>
#include <map>
#include <sstream>
#include <vector>

namespace ft {
template <typename T>
T stoi(const std::string &str) {
	char *endptr;
	errno      = 0;
	long value = std::strtol(str.c_str(), &endptr, 10);

	if (*endptr != '\0') {
		throw std::invalid_argument("Trailing characters found after value");
	} else if (errno == ERANGE || value < std::numeric_limits<T>::min() ||
	           value > std::numeric_limits<T>::max()) {
		throw std::out_of_range("Value out of range");
	}
	return static_cast<T>(value);
}

template <typename T>
std::string quoted(const T &element) {
	std::stringstream ss;

	ss << '"' << element << '"';
	return ss.str();
}

template <typename KEY, typename VAL>
typename std::map<KEY, std::vector<VAL> >::const_iterator isBound(
    std::map<KEY, std::vector<VAL> > &haystack, const VAL &needle) {
	for (typename std::map<KEY, std::vector<VAL> >::const_iterator it = haystack.begin();
	     it != haystack.end(); ++it) {
		for (typename std::vector<VAL>::const_iterator cmp = it->second.begin();
		     cmp != it->second.end(); ++cmp) {
			if (0 == std::memcmp(&needle, &(*cmp), sizeof(VAL))) {
				return it;
			}
		}
	}
	return haystack.end();
}

bool        is_dir(char *path);
std::string numToStr(int num);
std::string generateRandomString(size_t len);
}  // namespace ft

#endif
