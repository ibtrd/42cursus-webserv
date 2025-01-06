#ifndef REQUESTPOST_HPP
# define REQUESTPOST_HPP

# include "ARequest.hpp"

class RequestPOST : public ARequest {
private:

public:
	RequestPOST(Client &client);
	RequestPOST(const RequestPOST &other);

	~RequestPOST(void);

	RequestPOST	&operator=(const RequestPOST &other);

	error_t		process(void);
	ARequest	*clone(void) const;
};

ARequest	*createRequestPOST(Client &client);

#endif /* ******************************************************************* */
