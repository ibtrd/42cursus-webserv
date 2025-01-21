#ifndef REQUESTPUT_HPP
#define REQUESTPUT_HPP

#include <fstream>

#include "ARequest.hpp"

// # define CHUMK_TERMINATOR "0\r\n\r\n"
# define CHUNK_TERMINATOR_SIZE 5

class RequestPUT : public ARequest {
   private:
	static const char	*_chunkTerminator[CHUNK_TERMINATOR_SIZE];

	std::ofstream	_file;
	Path			_tmpFilename;

	bool	_chunked;
	int32_t	_contentLength;

	int32_t _contentTotalLength;

	error_t	_generateFilename(void);
	void	_openFile(void);
	error_t	_checkHeaders(void);
	void	_saveFile(void);
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
