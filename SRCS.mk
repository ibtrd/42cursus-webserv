SRCS_DIR = srcs/
SRC = \
	Configuration \
	main \
	Request \
	Response \
	Server \
	signal \
	utils \

SRCS := $(addsuffix .cpp, $(SRC))
