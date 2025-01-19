#ifndef REQUESTPUT_HPP
#define REQUESTPUT_HPP

#include <fstream>

#include "ARequest.hpp"

class RequestPUT : public ARequest {
   private:
	std::ofstream _file;

	void _openFile(const char *filepath);

	bool	_chunked;
	int32_t	_contentLength;

   public:
	RequestPUT(RequestContext_t &context);
	RequestPUT(const RequestPUT &other);

	~RequestPUT(void);

	RequestPUT &operator=(const RequestPUT &other);

	error_t   parse(void);
	error_t   processIn(void);
	error_t   processOut(void);
	ARequest *clone(void) const;
};

ARequest *createRequestPUT(RequestContext_t &context);

#endif /* ******************************************************************* */
