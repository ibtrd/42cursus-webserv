#ifndef REQUESTPOST_HPP
#define REQUESTPOST_HPP

#include <fstream>

#include "ARequest.hpp"

class RequestPOST : public ARequest {
private:
	std::ofstream _file;
	Path          _tmpFilename;

	bool    _chunked;
	int32_t _contentLength;

	int32_t _contentTotalLength;

	error_t _generateFilename(void);
	void    _openFile(void);
	void    _openCGI(void);
	error_t _checkHeaders(void);
	void    _saveFile(void);
	error_t _readContent(void);
	error_t _readChunked(void);

public:
	RequestPOST(RequestContext_t &context);
	RequestPOST(const RequestPOST &other);

	~RequestPOST(void);

	RequestPOST &operator=(const RequestPOST &other);

	void      processing(void);
	
	error_t   workIn(void);

	ARequest *clone(void) const;
};

ARequest *createRequestPOST(RequestContext_t &context);

#endif /* ******************************************************************* */
