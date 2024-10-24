SRCS_DIR = srcs/
SRC = \
	main \
	Server \
	Configuration \
	signal \
	Request \

SRCS := $(addsuffix .cpp, $(SRC))
