#ifndef REQUESTHEAD_HPP
#define REQUESTHEAD_HPP

#include <dirent.h>

#include <fstream>

#include "ARequest.hpp"

class RequestHEAD : public ARequest {
private:
	std::ifstream _file;
	DIR          *_dir;

	void _openFile(void);
	void _openDir(void);

	error_t _fetchIndexes(void);
	error_t _validateLocalFile(void);

public:
	RequestHEAD(RequestContext_t &context);
	RequestHEAD(const RequestHEAD &other);

	~RequestHEAD(void);

	RequestHEAD &operator=(const RequestHEAD &other);

	void processing(void);

	ARequest *clone(void) const;
};

ARequest *createRequestHEAD(RequestContext_t &context);

#endif /* ******************************************************************* */
