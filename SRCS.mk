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
	Configuration \
	Configuration-construction \
	Configuration-getters \
	ConfToken \
	ConfFile \
	ConfFile-errors \
	ConfFile-statics \
	ConfFile-locations \

SRCS := $(addsuffix .cpp, $(SRC))