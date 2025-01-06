#ifndef REQUESTGET_HPP
# define REQUESTGET_HPP

# include "ARequest.hpp"

class RequestGET : public ARequest {
private:

public:
	RequestGET(Client &client);
	RequestGET(const RequestGET &other);

	~RequestGET(void);

	RequestGET	&operator=(const RequestGET &other);

	error_t		process(void);
	ARequest	*clone(void) const;
};

ARequest	*createRequestGET(Client &client);

#endif /* ******************************************************************* */
