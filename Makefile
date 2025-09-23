NAME = pipex

CFLAGS = -Wall -Wextra -Werror

SRC = sources/pipex.c sources/pipex_utils.c

OBJ_DIR = objs/
OBJ = $(SRC:sources/%.c=$(OBJ_DIR)/%.o)

LIBFT_PATH = ./libft
LIBFT = $(LIBFT_PATH)/libft.a
FTINCL = -L libft/ -lft

WHITE	:= \033[0m
RED		:= \033[1;31m
GREEN	:= \033[1;32m
BLUE 	:= \033[1;34m

#########################################################

all:$(NAME)
	@echo "\n$(BLUE)Ready$(WHITE)\n"

$(NAME): $(LIBFT) $(OBJ_DIR) $(OBJ) 
	@cc $(CFLAGS) $(OBJ) -o $(NAME) $(FTINCL)
	@echo "$(GREEN)All compiled"

$(LIBFT):
	@$(MAKE) -C $(LIBFT_PATH) all -s
	@echo "$(GREEN)Libft compiled$(WHITE)"

$(OBJ_DIR)/%.o: sources/%.c
	@cc $(CFLAGS) -I libft -I  -I sources -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@$(MAKE) -C $(LIBFT_PATH) clean
	@rm -rf $(OBJ_DIR)
	@echo "$(RED)Objs removed$(WHITE)"

fclean:clean
	@$(MAKE) -C $(LIBFT_PATH) fclean
	@rm -rf $(NAME)
	@echo "$(RED)All clean$(WHITE)"

re:fclean all

.PHONY: all clean fclean re