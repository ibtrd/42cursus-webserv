#ifndef REQUESTPUT_HPP
#define REQUESTPUT_HPP

#include <fstream>

#include "ARequest.hpp"

class RequestPUT : public ARequest {
private:
	error_t _checkHeaders(void);

public:
	RequestPUT(RequestContext_t &context);
	RequestPUT(const RequestPUT &other);

	~RequestPUT(void);

	RequestPUT &operator=(const RequestPUT &other);

	void      processing(void);
	
	error_t   workIn(void);

	ARequest *clone(void) const;
};

ARequest *createRequestPUT(RequestContext_t &context);

#endif /* ******************************************************************* */
