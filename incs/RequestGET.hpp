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
	error_t		handle(void);
	error_t		parseRequest(void);
	error_t		parseBody(void);
	error_t		switchToWrite(void);
	error_t		sendResponse(void);
	error_t		readFile(void);
};

ARequest	*createRequestGET(void);

#endif /* ******************************************************************* */
