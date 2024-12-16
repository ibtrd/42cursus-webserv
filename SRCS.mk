SRCS_DIR = srcs/
SRC = \
	Configuration \
	main \
	Request \
	Server \
	ServerBlock \
	signal \
	utils \

SRCS := $(addsuffix .cpp, $(SRC))
