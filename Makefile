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


clean:
	rm -rf $(OBJ_DIR)


fclean: clean
	rm -f $(NAME) $(LINK)


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

.PHONY: all clean fclean re test test_rpath

