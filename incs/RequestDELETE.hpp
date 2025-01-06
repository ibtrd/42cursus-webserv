#ifndef REQUESTDELETE_HPP
# define REQUESTDELETE_HPP

# include "ARequest.hpp"

class RequestDELETE : public ARequest {
private:

public:
	RequestDELETE(void);
	RequestDELETE(const RequestDELETE &other);

	~RequestDELETE(void);

	RequestDELETE	&operator=(const RequestDELETE &other);

	ARequest	*clone(void) const;
};

ARequest	*createRequestDELETE(void);

#endif /* ******************************************************************* */
