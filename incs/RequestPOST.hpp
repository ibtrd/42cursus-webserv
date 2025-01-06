#ifndef REQUESTPOST_HPP
# define REQUESTPOST_HPP

# include "ARequest.hpp"

class RequestPOST : public ARequest {
private:

public:
	RequestPOST(void);
	RequestPOST(const RequestPOST &other);

	~RequestPOST(void);

	RequestPOST	&operator=(const RequestPOST &other);

	ARequest	*clone(void) const;
};

ARequest	*createRequestPOST(void);

#endif /* ******************************************************************* */
