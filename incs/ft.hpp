#ifndef FT_HPP
# define FT_HPP

# include <string>
# include <cstring>
# include <sstream>
# include <stdexcept>

namespace ft {
	template<typename T>
	T stoi(const std::string& str) {
		std::stringstream ss(str);
		T value;

		if (!(ss >> value) || !ss.eof()) {
			throw std::invalid_argument("Failed to convert value");
		}
		if (ss.peek() != std::stringstream::traits_type::eof()) {
			throw std::invalid_argument("Trailing characters found after value");
		}
		return value;
	}

	template<typename T>
	std::string quoted(const T &element) {
		std::stringstream ss;

		ss << '"' << element << '"';
		return ss.str();
	}

	template<typename KEY, typename VAL>
	typename std::map<KEY, std::vector<VAL> >::const_iterator isBound(std::map<KEY, std::vector<VAL> > &haystack, const VAL &needle) {
	for(typename std::map<KEY, std::vector<VAL> >::const_iterator it = haystack.begin(); it != haystack.end(); ++it) {
		for (typename std::vector<VAL>::const_iterator cmp = it->second.begin(); cmp != it->second.end(); ++cmp) {
			if (0 == std::memcmp(&needle, &(*cmp), sizeof(VAL))) {
				return it;
			}
		}
	}
	return haystack.end();
}

	bool		is_dir(char *path);
	std::string	numToStr(int num);
}

#endif
