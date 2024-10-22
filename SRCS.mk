SRCS_DIR = srcs/
SRC = \
	Client \
	main \
	Server \

SRCS := $(addsuffix .cpp, $(SRC))
