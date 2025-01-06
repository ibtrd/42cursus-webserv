#ifndef AREQUEST_HPP
# define AREQUEST_HPP

# include "webdef.hpp"

# include <string>
# include <map>

# define REQ_BUFFER_SIZE 1024

# define REQ_CONTINUE 0	// Incomplete task, skip to next call
# define REQ_DONE 1		// Done with current task
# define REQ_ERROR 2	// Program error

# define REQ_STATE_NONE							0x00000000
# define REQ_STATE_READ_REQUEST_LINE_COMPLETE	0x00000001
# define REQ_STATE_READ_HEADERS_COMPLETE		0x00000002
# define REQ_STATE_READ_BODY_COMPLETE			0x00000004
# define REQ_STATE_CLIENT_READ_COMPLETE			0x00000008
# define REQ_STATE_READ_COMPLETE				0x0000000F
# define REQ_STATE_PROCESS_COMPLETE				0x00000010
# define REQ_STATE_CAN_WRITE					0x00000020
# define REQ_STATE_WRITE_COMPLETE				0x00000040

# define IS_REQ_READ_REQUEST_LINE_COMPLETE(x)	((x & REQ_STATE_READ_REQUEST_LINE_COMPLETE) == REQ_STATE_READ_REQUEST_LINE_COMPLETE)
# define IS_REQ_READ_HEADERS_COMPLETE(x)		((x & REQ_STATE_READ_HEADERS_COMPLETE) == REQ_STATE_READ_HEADERS_COMPLETE)
# define IS_REQ_READ_BODY_COMPLETE(x)			((x & REQ_STATE_READ_BODY_COMPLETE) == REQ_STATE_READ_BODY_COMPLETE)
# define IS_REQ_CLIENT_READ_COMPLETE(x)			((x & REQ_STATE_CLIENT_READ_COMPLETE) == REQ_STATE_CLIENT_READ_COMPLETE)
# define IS_REQ_READ_COMPLETE(x)				((x & REQ_STATE_READ_COMPLETE) == REQ_STATE_READ_COMPLETE)
# define IS_REQ_PROCESS_COMPLETE(x)				((x & REQ_STATE_PROCESS_COMPLETE) == REQ_STATE_PROCESS_COMPLETE)
# define IS_REQ_CAN_WRITE(x)					((x & REQ_STATE_CAN_WRITE) == REQ_STATE_CAN_WRITE)
# define IS_REQ_WRITE_COMPLETE(x)				((x & REQ_STATE_WRITE_COMPLETE) == REQ_STATE_WRITE_COMPLETE)

# define SET_REQ_READ_REQUEST_LINE_COMPLETE(x)	(x |= REQ_STATE_READ_REQUEST_LINE_COMPLETE)
# define SET_REQ_READ_HEADERS_COMPLETE(x)		(x |= REQ_STATE_READ_HEADERS_COMPLETE)
# define SET_REQ_READ_BODY_COMPLETE(x)			(x |= REQ_STATE_READ_BODY_COMPLETE)
# define SET_REQ_CLIENT_READ_COMPLETE(x)		(x |= REQ_STATE_CLIENT_READ_COMPLETE)
# define SET_REQ_READ_COMPLETE(x)				(x |= REQ_STATE_READ_COMPLETE)
# define SET_REQ_PROCESS_COMPLETE(x)			(x |= REQ_STATE_PROCESS_COMPLETE)
# define SET_REQ_CAN_WRITE(x)					(x |= REQ_STATE_CAN_WRITE)
# define SET_REQ_WRITE_COMPLETE(x)				(x |= REQ_STATE_WRITE_COMPLETE)

class Client;

class ARequest {
protected:
	Client	&_client;

public:
	ARequest(Client &client);
	ARequest(const ARequest &other);

	virtual ~ARequest(void);

	ARequest	&operator=(const ARequest &other);

	virtual error_t	process(void) = 0;

	virtual ARequest	*clone(void) const = 0;
};

#endif /* ******************************************************************* */
