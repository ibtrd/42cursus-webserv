# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ibertran <ibertran@student.42lyon.fr>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/08/13 22:47:55 by ibertran          #+#    #+#              #
#    Updated: 2025/01/21 17:10:33 by ibertran         ###   ########lyon.fr    #
#                                                                              #
# **************************************************************************** #

NAME = webserv

# *** FILES ****************************************************************** #

MAKE_DIR := .make/
BUILD_DIR := $(MAKE_DIR)$(shell git branch --show-current 2>/dev/null)/

include SRCS.mk

OBJS = $(patsubst %.cpp,$(BUILD_DIR)%.o,$(SRCS))

DEPS = $(patsubst %.o,%.d,$(OBJS))

# *** LIBRARIES && INCLUDES  ************************************************* #

INCS_DIR = incs/
INCS = \
	$(INCS_DIR) \

# *** CONFIG ***************************************************************** #

CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98

DEFINES		=

CPPFLAGS 	=	$(addprefix -I, $(INCS)) \
				$(addprefix -D, $(DEFINES)) \
				-MD -MP \

MAKEFLAGS	=	--no-print-directory

# *** MODES ****************************************************************** #

MODE_TRACE = $(MAKE_DIR).trace 
LAST_MODE = $(shell cat $(MODE_TRACE) 2>/dev/null)

ifneq ($(MODE),)
BUILD_DIR := $(BUILD_DIR)$(MODE)/
endif

ifeq ($(MODE),debug)
CXXFLAGS := $(CXXFLAGS) -g3
else ifeq ($(MODE),fsanitize)
CXXFLAGS := $(CXXFLAGS) -g3 -fsanitize=address
else ifneq  ($(MODE),)
ERROR = MODE
endif

ifneq ($(LAST_MODE),$(MODE))
$(NAME) : FORCE
endif

# *** TARGETS **************************************************************** #

-include $(DEPS)

.DEFAULT_GOAL := all

.PHONY : all
all : $(NAME)

$(NAME) : $(OBJS)
	@echo "$(BOLD)$(CXX) $(CYAN)$(OBJS) $(WHITE)-o $(NAME)$(RESET)"
	@$(CXX) $(OBJS) -o $(NAME)
	@echo "$(MODE)" > $(MODE_TRACE)

$(BUILD_DIR)%.o : $(SRCS_DIR)%.cpp | MODE_CHECK
	@mkdir -p $(@D)
	@echo "$(BOLD)$(CXX) $(YELLOW)$(CXXFLAGS) $(CPPFLAGS) $(BLUE)-c $< $(CYAN)-o $@$(RESET)"
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY : debug
debug :
	$(MAKE) MODE=debug

.PHONY : fsanitize
fsanitize :
	$(MAKE) MODE=fsanitize

.PHONY : clean
clean :
	rm -rf $(BUILD_DIR)

.PHONY : fclean
fclean :
	rm -rf $(MAKE_DIR) $(NAME)

.PHONY : re
re : fclean
	$(MAKE)

.PHONY : run
run : $(NAME)
	./$(NAME)

CLANG_FORMAT_SOURCES := $(addprefix $(SRCS_DIR), $(SRCS)) $(shell find $(INCS_DIR) -type f -name '*.hpp')

.PHONY : check-format
check-format :
	clang-format --Werror --dry-run $(CLANG_FORMAT_SOURCES)

.PHONY : clang-format
clang-format :
	clang-format -i $(CLANG_FORMAT_SOURCES)

.PHONY : print-%
print-% :
	@echo $(patsubst print-%,%,$@)=
	@echo $($(patsubst print-%,%,$@))

.PHONY : MODE_CHECK
MODE_CHECK :
ifeq ($(ERROR),MODE)
	$(error Invalid mode: $(MODE))
endif

.PHONY : FORCE
FORCE :

# *** FANCY STUFF ************************************************************ #

RESET	=	\e[0m
ERASE	=	\033[2K\r
BOLD	=	\033[1m
UNDER	=	\033[4m
SUR		=	\033[7m
GREY	=	\033[30m
RED		=	\033[31m
GREEN	=	\033[32m
YELLOW	=	\033[33m
BLUE	=	\033[34m
PURPLE	=	\033[35m
CYAN	=	\033[36m
WHITE	=	\033[37m
C12		=	\033[39m
C13		=	\033[43m
