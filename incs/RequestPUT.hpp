#ifndef REQUESTPUT_HPP
#define REQUESTPUT_HPP

#include <fstream>

#include "ARequest.hpp"

class RequestPUT : public ARequest {
   private:
	std::ofstream	_file;
	Path			_tmpFilename;

	bool	_chunked;
	int32_t	_contentLength;

	error_t	_generateFilename(void);
	void	_openFile(void);
	error_t	_checkHeaders(void);
	error_t	_readContent(void);
	error_t	_readChunked(void);

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
