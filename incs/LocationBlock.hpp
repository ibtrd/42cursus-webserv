#ifndef LOCATIONBLOCK_HPP
# define LOCATIONBLOCK_HPP


# include "Path.hpp"
# include "webdef.hpp"

# define DEFAULT_METHODS 0
# define DEFAULT_DIRLISTING false
# define DEFAULT_MAXBODYSIZE -1

typedef std::pair<uint16_t, std::string> redirect_t;

class LocationBlock {
public:
	LocationBlock(void);
	LocationBlock(const LocationBlock &other);
	LocationBlock(const Path &path);

	~LocationBlock(void);

	LocationBlock	&operator=(const LocationBlock &other);

	bool	match(const Path &target) const;

	// SETTERS
	error_t	allowMethod(const std::string &str);
	error_t	setDirListing(const std::string &str);
	void	setMaxBodySize(const int32_t size);
	error_t	setRoot(const std::string &str);
	void	setRedirect(const uint16_t status, const std::string &body);

	// GETTERS
	const Path			&path(void) const;
	bool				isAllowed(const std::string &method) const;
	bool				isDirListing(void) const;
	int32_t				getMaxBodySize(void) const;
	const Path		 	&getRoot(void) const;
	const redirect_t 	&getRedirect(void) const;

private:
	Path		_path;
	bool		_dirListing;
	int32_t		_maxBodySize;
	Path		_root;
	uint8_t		_allowed;
	redirect_t	_redirection;

	// STATICS
	static const std::vector<std::string>	_methods;

	static std::vector<std::string>	_initMethods(void);
};

#endif /* ******************************************************************* */
