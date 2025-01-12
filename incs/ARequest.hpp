#ifndef AREQUEST_HPP
# define AREQUEST_HPP

# include "RequestContext.hpp"

# include <vector>

class ARequest {
protected:
	RequestContext_t	&_context;

public:
	ARequest(RequestContext_t &context);
	ARequest(const ARequest &other);

	virtual ~ARequest(void);

	ARequest	&operator=(const ARequest &other);

	virtual error_t	parse(void) = 0;
	virtual error_t	process(void) = 0;

	virtual ARequest	*clone(void) const = 0;
};

#endif /* ******************************************************************* */
