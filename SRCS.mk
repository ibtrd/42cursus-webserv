SRCS_DIR = srcs/
SRC = \
	main \
	Webserv \
	Configuration \
	signal \

SRCS := $(addsuffix .cpp, $(SRC))
