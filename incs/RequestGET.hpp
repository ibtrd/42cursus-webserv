#ifndef REQUESTGET_HPP
#define REQUESTGET_HPP

#include <dirent.h>

#include <fstream>

#include "ARequest.hpp"

class RequestGET : public ARequest {
private:
	std::ifstream _file;
	DIR          *_dir;

	void _openFile(void);
	void _openDir(void);

	error_t _readFile(void);
	error_t _readDir(void);

	error_t _fetchIndexes(void);
	error_t _validateLocalFile(void);

public:
	RequestGET(RequestContext_t &context);
	RequestGET(const RequestGET &other);

	~RequestGET(void);

	RequestGET &operator=(const RequestGET &other);

	void      processing(void);
	error_t   workIn(void);
	error_t   workOut(void);
	ARequest *clone(void) const;
};

ARequest *createRequestGET(RequestContext_t &context);

#endif /* ******************************************************************* */
