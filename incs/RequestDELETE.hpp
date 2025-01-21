#ifndef REQUESTDELETE_HPP
#define REQUESTDELETE_HPP

#include "ARequest.hpp"

class RequestDELETE : public ARequest {
private:
public:
	RequestDELETE(RequestContext_t &context);
	RequestDELETE(const RequestDELETE &other);

	~RequestDELETE(void);

	RequestDELETE &operator=(const RequestDELETE &other);

	void      processing(void);
	error_t   workIn(void);
	error_t   workOut(void);
	ARequest *clone(void) const;
};

ARequest *createRequestDELETE(RequestContext_t &context);

#endif /* ******************************************************************* */
