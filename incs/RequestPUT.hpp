#ifndef REQUESTPUT_HPP
# define REQUESTPUT_HPP

# include "ARequest.hpp"

class RequestPUT : public ARequest {
private:

public:
	RequestPUT(RequestContext_t &context);
	RequestPUT(const RequestPUT &other);

	~RequestPUT(void);

	RequestPUT	&operator=(const RequestPUT &other);

	error_t		parse(void);
	error_t		processIn(void);
	error_t		processOut(void);
	ARequest	*clone(void) const;
};

ARequest	*createRequestPUT(RequestContext_t &context);

#endif /* ******************************************************************* */
