#ifndef AREQUEST_HPP
#define AREQUEST_HPP

#include <vector>

#include "RequestContext.hpp"

// # define CHUMK_TERMINATOR "0\r\n\r\n"
#define CHUNK_TERMINATOR_SIZE 5

class ARequest {
protected:
	static const char *_chunkTerminator[CHUNK_TERMINATOR_SIZE];

	RequestContext_t &_context;
	Path              _path;
	const Path		  *_cgiPath;

public:
	ARequest(RequestContext_t &context);
	ARequest(const ARequest &other);

	virtual ~ARequest(void);

	ARequest &operator=(const ARequest &other);

	virtual void    processing(void) = 0;
	virtual error_t workIn(void);
	virtual error_t workOut(void);
	virtual error_t CGIIn(void);
	virtual error_t CGIOut(void);

	// bool	pathExists(void) const;

	virtual ARequest *clone(void) const = 0;
};

#endif /* ******************************************************************* */
