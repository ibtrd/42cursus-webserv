#ifndef AREQUEST_HPP
#define AREQUEST_HPP

#include <fstream>
#include <vector>

#include "RequestContext.hpp"

#define CHUNK_TERMINATOR_SIZE 5

class ARequest {
protected:
	static const char *_chunkTerminator[CHUNK_TERMINATOR_SIZE];

	RequestContext_t &_context;
	Path              _path;
	const Path       *_cgiPath;

	std::ofstream _file;
	Path          _tmpFilename;

	bool    _chunked;
	int32_t _contentLength;

	int32_t _contentTotalLength;

	BinaryBuffer _readBuffer;

	error_t _readCGI(void);
	void    _parseCGIHeaders(void);

	error_t _generateFilename(void);
	void    _openFile(void);

	void    _saveFile(void);
	error_t _writeChunk(size_t size);
	error_t _readContent(void);
	error_t _readChunked(void);

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

	const RequestContext_t &context(void) const;
	const Path             &path(void) const;
	const Path             &cgiPath(void) const;

	virtual ARequest *clone(void) const = 0;
};

#endif /* ******************************************************************* */
