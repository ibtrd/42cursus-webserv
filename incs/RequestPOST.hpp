#ifndef REQUESTPOST_HPP
# define REQUESTPOST_HPP

# include "ARequest.hpp"

class RequestPOST : public ARequest {
private:

public:
	RequestPOST(RequestContext_t &context);
	RequestPOST(const RequestPOST &other);

	~RequestPOST(void);

	RequestPOST	&operator=(const RequestPOST &other);

	error_t		parse(void);
	error_t		process(void);
	ARequest	*clone(void) const;
};

ARequest	*createRequestPOST(RequestContext_t &context);

#endif /* ******************************************************************* */
