SRCS_DIR = srcs/
SRC = \
	main \
	Server \
	Configuration \
	signal \

SRCS := $(addsuffix .cpp, $(SRC))
