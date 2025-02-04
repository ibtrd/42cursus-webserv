#ifndef REQUESTPOST_HPP
#define REQUESTPOST_HPP

#include <fstream>

#include "ARequest.hpp"

class RequestPOST : public ARequest {
private:
	void _openCGI(void);

	error_t _executeCGI(void);

	error_t _checkHeaders(void);
	error_t _validateLocalFile(void);

public:
	RequestPOST(RequestContext_t &context);
	RequestPOST(const RequestPOST &other);

	~RequestPOST(void);

	RequestPOST &operator=(const RequestPOST &other);

	void processing(void);

	error_t workIn(void);
	error_t workOut(void);

	error_t CGIIn(void);
	error_t CGIOut(void);

	ARequest *clone(void) const;
};

ARequest *createRequestPOST(RequestContext_t &context);

#endif /* ******************************************************************* */
