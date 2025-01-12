#ifndef REQUESTGET_HPP
# define REQUESTGET_HPP

# include "ARequest.hpp"

class RequestGET : public ARequest {
private:
	fd_t	_fd;

	void	_openFile(void);

public:
	RequestGET(RequestContext_t &context);
	RequestGET(const RequestGET &other);

	~RequestGET(void);

	RequestGET	&operator=(const RequestGET &other);

	error_t		parse(void);
	error_t		process(void);
	ARequest	*clone(void) const;
};

ARequest	*createRequestGET(RequestContext_t &context);

#endif /* ******************************************************************* */
