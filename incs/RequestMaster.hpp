#ifndef REQUESTMASTER_HPP
# define REQUESTMASTER_HPP

# include "ARequest.hpp"

class RequestMaster : public ARequest {
private:

public:
	RequestMaster(void);
	RequestMaster(const RequestMaster &other);

	~RequestMaster(void);

	RequestMaster	&operator=(const RequestMaster &other);

	error_t		init(const int32_t requestSocket);
	error_t		handle(void);
	error_t		parseRequest(void);
	error_t		parseRequestLine(void);
	error_t		parseHeaders(void);
	error_t		switchToWrite(void);
	error_t		sendResponse(void);
};

#endif /* ******************************************************************* */
