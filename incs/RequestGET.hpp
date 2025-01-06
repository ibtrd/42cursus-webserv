#ifndef REQUESTGET_HPP
# define REQUESTGET_HPP

# include "ARequest.hpp"

class RequestGET : public ARequest {
private:

public:
	RequestGET(void);
	RequestGET(const RequestGET &other);

	~RequestGET(void);

	RequestGET	&operator=(const RequestGET &other);

	ARequest	*clone(void) const;
};

ARequest	*createRequestGET(void);

#endif /* ******************************************************************* */
