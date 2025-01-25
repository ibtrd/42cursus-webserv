#ifndef REQUESTGET_HPP
#define REQUESTGET_HPP

#include <dirent.h>

#include <fstream>

#include "ARequest.hpp"

class RequestGET : public ARequest {
private:
	static const char *_chunkTerminator[CHUNK_TERMINATOR_SIZE];

	std::ifstream _file;
	DIR          *_dir;
	pid_t         _pid;

	void _openFile(void);
	void _openDir(void);
	void _openCGI(void);

	error_t _readFile(void);
	error_t _readDir(void);

	error_t _executeCGI(void);

	error_t _fetchIndexes(void);
	error_t _validateLocalFile(void);

public:
	RequestGET(RequestContext_t &context);
	RequestGET(const RequestGET &other);

	~RequestGET(void);

	RequestGET &operator=(const RequestGET &other);

	void      processing(void);

	error_t   workOut(void);
	error_t   CGIIn(void);

	ARequest *clone(void) const;
};

ARequest *createRequestGET(RequestContext_t &context);

#endif /* ******************************************************************* */
