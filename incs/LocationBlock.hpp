#ifndef LOCATIONBLOCK_HPP
# define LOCATIONBLOCK_HPP

# include <stdint.h>
# include <string>
# include <vector>

# define GET_MASK (1 << 0)
# define POST_MASK (1 << 1)
# define DELETE_MASK (1 << 2)

# define DEFAULT_METHODS 0
# define DEFAULT_DIRLISTING false
# define DEFAULT_MAXBODYSIZE -1

class LocationBlock {
public:
	LocationBlock(void);
	LocationBlock(const LocationBlock &other);
	LocationBlock(const std::string &path);

	~LocationBlock(void);

	LocationBlock	&operator=(const LocationBlock &other);

	// SETTERS
	bool	allowMethod(const std::string &str);
	bool	setDirListing(const std::string &str);
	void	setMaxBodySize(const int32_t size);
	void	setRoot(const std::string &str);

	// GETTERS
	const std::string	&path(void) const;
	bool				isAllowedMethod(const std::string &str) const;
	bool				isDirListing(void) const;
	int32_t				getMaxBodySize(void) const;
	const std::string 	&getRoot(void) const;


private:
	std::string	_path;
	bool		_dirListing;
	int32_t		_maxBodySize;
	std::string	_root;
	uint8_t		_allowedMethods;

	// STATICS
	static const std::vector<std::string>	_methods;

	static std::vector<std::string>	_initMethods(void);
};

#endif /* ******************************************************************* */
