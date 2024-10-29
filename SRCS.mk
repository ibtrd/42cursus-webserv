SRCS_DIR = srcs/
SRC = \
	main \
	Server \
	Configuration \
	signal \
	Request \
	utils \

SRCS := $(addsuffix .cpp, $(SRC))
