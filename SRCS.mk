SRCS_DIR = srcs/
SRC = \
	main \
	Request \
	Server \
	ServerBlock \
	signal \
	utils \
	LocationBlock \

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

SRCS := $(addsuffix .cpp, $(SRC))