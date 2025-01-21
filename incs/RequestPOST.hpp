#ifndef REQUESTPOST_HPP
#define REQUESTPOST_HPP

#include "ARequest.hpp"

class RequestPOST : public ARequest {
   private:
   public:
	RequestPOST(RequestContext_t &context);
	RequestPOST(const RequestPOST &other);

	~RequestPOST(void);

	RequestPOST &operator=(const RequestPOST &other);

	void		processing(void);
	error_t		workIn(void);
	error_t		workOut(void);
	ARequest	*clone(void) const;
};

ARequest *createRequestPOST(RequestContext_t &context);

#endif /* ******************************************************************* */
