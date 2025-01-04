#ifndef FT_HPP
# define FT_HPP

# include <string>
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

	bool		is_dir(char *path);
	std::string	numToStr(int num);
}

#endif
