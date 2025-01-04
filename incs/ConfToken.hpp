#ifndef CONFTOKEN_HPP
# define CONFTOKEN_HPP

# include <string>
# include <stdint.h>

# define BLOCK_OPEN '{'
# define BLOCK_CLOSE '}'

class ConfToken {
public:
	ConfToken(void);
	ConfToken(const ConfToken &other);
	ConfToken(const std::string &str, const uint32_t line);

	~ConfToken(void);

	ConfToken	&operator=(const ConfToken &other);

	bool	operator==(const std::string &str) const;
	bool	operator==(const char c) const;
	bool	operator!=(const std::string &str) const;
	bool	operator!=(const char c) const;

	const std::string	&str(void) const;
	uint32_t			line(void) const;

	bool	isMetatoken(void) const;

private:
	std::string	_str;
	uint32_t	_line;
};

#endif /* ******************************************************************* */
