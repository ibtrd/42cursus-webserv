#ifndef AREQUEST_HPP
#define AREQUEST_HPP

#include <vector>

#include "RequestContext.hpp"

class ARequest {
   protected:
	RequestContext_t &_context;
	Path              _path;

   public:
	ARequest(RequestContext_t &context);
	ARequest(const ARequest &other);

	virtual ~ARequest(void);

	ARequest &operator=(const ARequest &other);

	virtual error_t parse(void)      = 0;
	virtual error_t processIn(void)  = 0;
	virtual error_t processOut(void) = 0;

	// bool	pathExists(void) const;

	virtual ARequest *clone(void) const = 0;
};

#endif /* ******************************************************************* */
