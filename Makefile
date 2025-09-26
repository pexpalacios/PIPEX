MAKEFLAGS += --no-print-directory

NAME = pipex
CC	= cc
CFLAGS = -Wall -Wextra -Werror

SRCS = pip.c

OBJ_DIR = objs/
OBJ = $(SRCS:%.c=$(OBJ_DIR)%.o)

LIBFT_PATH = ./libft
LIBFT = $(LIBFT_PATH)/libft.a
FTINCL = -L libft/ -lft

BLUE = \033[1;34m
YELLOW = \033[1;33m
GREEN = \033[1;32m
RESET = \033[0m

####################

all:$(NAME)
                               
$(NAME): $(LIBFT) $(OBJ_DIR) $(OBJ) 
	@cc $(CFLAGS) $(OBJ) -o $(NAME) $(FTINCL)
	@echo "$(GREEN) _____ _____ _____ _____ __ __ "
	@echo "|  _  |     |  _  |   __|  |  |"
	@echo "|   __|>   <|   __|   __|>   < "
	@echo "|__|  |_____|__|  |_____|__|__|"
	@echo "$(BLUE)                   by penpalac$(RESET)"

$(LIBFT):
	@$(MAKE) -C $(LIBFT_PATH) all -s

$(OBJ_DIR)%.o: %.c | $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@$(MAKE) -C $(LIBFT_PATH) clean
	@rm -rf $(OBJ_DIR)
	@echo "$(YELLOW)[Objs removed]$(RESET)"

fclean:clean
	@$(MAKE) -C $(LIBFT_PATH) fclean
	@rm -rf $(NAME)
	@echo "$(YELLOW)[$(NAME) removed]$(RESET)"

re: fclean all

.PHONY: all clean fclean re
