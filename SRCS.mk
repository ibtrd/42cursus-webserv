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

SRC += $(addprefix $(CONFIGURATION_DIR), $(CONFIGURATION_SRC))
CONFIGURATION_DIR=Configuration/

CONFIGURATION_SRC = \
	ConfFile \
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

SRCS := $(addsuffix .cpp, $(SRC))