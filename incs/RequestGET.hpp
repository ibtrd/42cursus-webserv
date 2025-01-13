#ifndef REQUESTGET_HPP
# define REQUESTGET_HPP

# include <fstream>

# include "ARequest.hpp"

class RequestGET : public ARequest {
private:
	// fd_t	_fd;
	std::ifstream	_file;

	void	_openFile(void);

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
