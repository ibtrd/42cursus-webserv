SRCS_DIR = srcs/
SRC = \
	Client \
	ft \
	LocationBlock \
	main \
	Response \
	Server \
	ServerBlock \
	signal \
	utils \
	Path \
	Method \

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

SRCS := $(addsuffix .cpp, $(SRC))