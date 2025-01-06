#ifndef REQUESTDELETE_HPP
# define REQUESTDELETE_HPP

# include "ARequest.hpp"

class RequestDELETE : public ARequest {
private:

public:
	RequestDELETE(Client &client);
	RequestDELETE(const RequestDELETE &other);

	~RequestDELETE(void);

	RequestDELETE	&operator=(const RequestDELETE &other);

	error_t		process(void);
	ARequest	*clone(void) const;
};

ARequest	*createRequestDELETE(Client &client);

#endif /* ******************************************************************* */
