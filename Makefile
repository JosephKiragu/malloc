# **************************************************************************** #
#                                                                              #
#                                         :::      ::::::::::::               #
#    Makefile                           :+:      :+:    :+:                   #
#                                     +:+ +:+         +:+                     #
#    By: Joseph <jkiragu@42.adl>      +#+  +:+       +#+                      #
#                                 +#+#+#+#+#+   +#+                           #
#    Created: 2025-05             by Joseph        #+#    #+#                #
#    Updated: 2025-05             by Joseph        ###   ########.fr         #
#                                                                              #
# **************************************************************************** #

ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME = libft_malloc_$(HOSTTYPE).so
LINK = libft_malloc.so

CC = gcc
CFLAGS = -Wall -Wextra -Werror -fPIC
LDFLAGS = -shared -pthread

ifdef DEBUG
	CFLAGS += -DDEBUG -g -O0
	NAME = libft_malloc_$(HOSTTYPE)_debug.so
else
	CFLAGS += -O2 -DNDEBUG
endif


SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj


SRCS =	$(SRC_DIR)/malloc.c \
		$(SRC_DIR)/free.c \
		$(SRC_DIR)/realloc.c \
		$(SRC_DIR)/show_alloc.c \
		$(SRC_DIR)/zones.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
INCS = $(INC_DIR)/malloc.h

all: $(NAME)


$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@


$(NAME): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
	ln -sf $(NAME) $(LINK)


debug:
	$(MAKE) DEBUG=1


clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f libft_malloc_$(HOSTTYPE)*.so $(LINK)


re: fclean all

# comment out the testing section when done with testing
# Testing program
TEST = test_malloc
TEST_SRC = test.c

# compiling test program
$(TEST): $(NAME) $(TEST_SRC)
	$(CC) $(CFLAGS) -I$(INC_DIR) $(TEST_SRC) -L. -lft_malloc -Wl,-rpath,. -o $(TEST)

# running test
test: $(TEST)
	LD_LIBRARY_PATH=. ./$(TEST)

# alternative run test without LD_LIBRARY_PATH (if RPATH is embedded)
test_rpath: $(TEST)
	./$(TEST)

# debug test 
test_debug:
	$(MAKE) DEBUG=1 test

test_debug_existing:
	@if [ ! -f libft_malloc_$(HOSTTYPE)_debug.so ]; then \
		echo "Debug library not found. BUilding..."; \
		$(MAKE) DEBUG=1; \
	fi
	ln -sf libft_malloc_$(HOSTTYPE)_debug.so $(LINK)
	$(MAKE) DEBUG=1 $(TEST)
	LD_LIBRARY_PATH=. ./$(TEST)

# display current build configuration
config:
	@echo "Build configuration:"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "Library: $(NAME)"
	@echo "Debug mode: $(if $(DEBUG),ENABLED,DISABLED)"

.PHONY: all debug clean fclean re test test_rpath test_debug config

