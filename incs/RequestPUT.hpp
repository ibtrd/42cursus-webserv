#ifndef REQUESTPUT_HPP
#define REQUESTPUT_HPP

#include <fstream>

#include "ARequest.hpp"

class RequestPUT : public ARequest {
   private:
	std::ofstream _file;

	bool	_chunked;
	int32_t	_contentLength;

	void	_openFile(const char *filepath);
	error_t	_checkHeaders(void);

   public:
	RequestPUT(RequestContext_t &context);
	RequestPUT(const RequestPUT &other);

	~RequestPUT(void);

	RequestPUT &operator=(const RequestPUT &other);

	void		processing(void);
	error_t		workIn(void);
	error_t		workOut(void);
	ARequest	*clone(void) const;
};

ARequest *createRequestPUT(RequestContext_t &context);

#endif /* ******************************************************************* */
