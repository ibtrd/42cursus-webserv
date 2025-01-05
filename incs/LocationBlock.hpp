#ifndef LOCATIONBLOCK_HPP
# define LOCATIONBLOCK_HPP

# include <stdint.h>
# include <string>
# include <vector>

# include "webdef.hpp"

# define GET_MASK (1 << 0)
# define POST_MASK (1 << 1)
# define DELETE_MASK (1 << 2)

# define DEFAULT_METHODS 0
# define DEFAULT_DIRLISTING false
# define DEFAULT_MAXBODYSIZE -1

typedef std::pair<uint16_t, std::string> redirect_t;

class LocationBlock {
public:
	LocationBlock(void);
	LocationBlock(const LocationBlock &other);
	LocationBlock(const std::string &path);

	~LocationBlock(void);

	LocationBlock	&operator=(const LocationBlock &other);

	// SETTERS
	error_t	allowMethod(const std::string &str);
	error_t	setDirListing(const std::string &str);
	void	setMaxBodySize(const int32_t size);
	void	setRoot(const std::string &str);
	void	setRedirect(const uint16_t status, const std::string &body);

	// GETTERS
	const std::string	&path(void) const;
	bool				isAllowedMethod(const std::string &str) const;
	bool				isDirListing(void) const;
	int32_t				getMaxBodySize(void) const;
	const std::string 	&getRoot(void) const;
	const redirect_t 	&getRedirect(void) const;


private:
	std::string	_path;
	bool		_dirListing;
	int32_t		_maxBodySize;
	std::string	_root;
	uint8_t		_allowedMethods;
	redirect_t	_redirection;

	// STATICS
	static const std::vector<std::string>	_methods;

	static std::vector<std::string>	_initMethods(void);
};

#endif /* ******************************************************************* */
