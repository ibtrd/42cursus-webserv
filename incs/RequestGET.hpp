#ifndef REQUESTGET_HPP
# define REQUESTGET_HPP

# include <fstream>
# include <dirent.h>

# include "ARequest.hpp"

class RequestGET : public ARequest {
private:
	std::ifstream	_file;
	DIR				*_dir;

	void	_openFile(void);
	void	_openDir(void);

	error_t	_readFile(void);
	error_t	_readDir(void);

public:
	RequestGET(RequestContext_t &context);
	RequestGET(const RequestGET &other);

	~RequestGET(void);

	RequestGET	&operator=(const RequestGET &other);

	error_t		parse(void);
	error_t		processIn(void);
	error_t		processOut(void);
	ARequest	*clone(void) const;
};

ARequest	*createRequestGET(RequestContext_t &context);

#endif /* ******************************************************************* */
