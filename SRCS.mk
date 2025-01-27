SRCS_DIR = srcs/
SRC = \
	BinaryBuffer \
	CgiBuilder \
	ft \
	LocationBlock \
	main \
	Method \
	Path \
	Queries \
	Response \
	Server \
	ServerBlock \
	signal \
	utils \

SRC += $(addprefix $(CONFIGURATION_DIR), $(CONFIGURATION_SRC))
CONFIGURATION_DIR=Configuration/

CONFIGURATION_SRC = \
	ConfFile \
	ConfFile-directives \
	ConfFile-errors \
	ConfFile-locations \
	ConfFile-servers \
	ConfFile-statics \
	Configuration \
	ConfToken \

SRC += $(addprefix $(REQUEST_DIR), $(REQUEST_SRC))
REQUEST_DIR=Request/

REQUEST_SRC = \
	ARequest \
	RequestGET \
	RequestPOST \
	RequestDELETE \
	RequestPUT \
	RequestGET-autoindex \
	RequestHEAD \
	RequestGET-cgi \
	ARequest-cgi \

SRC += $(addprefix $(CLIENT_DIR), $(CLIENT_SRC))
CLIENT_DIR=Client/

CLIENT_SRC = \
	Client \
	Client-errorPages \
	Client-parse \

SRCS := $(addsuffix .cpp, $(SRC))