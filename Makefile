## GLOBAL VARIABLES

NAME = minishell
SRCS = main.c
SRCS_FILES = $(addprefix src/, $(SRCS))
BUILTINS_FILES = cd.c export.c pwd.c echo.c env.c unset.c exit.c export_utils.c
SIGNAL_FILES = handle_signal.c
LEXER_FILES = tokenizer.c token_creation.c token_utils.c fd_handling.c
CHECK_TOKENS_FILES = utils.c analyze_tokens.c
EXPANDER_FILES = expander.c expand_helpers.c expander_utils.c expand_token_utils.c expand_token.c process_word.c
HANDLE_HEREDOC_FILES = heredoc.c heredoc_utils_1.c heredoc_utils.c
HANDLE_INPUT_FILES = process_line_utils.c process_input.c
INIT_FILES = init_data.c
PARSER_FILES = free_ast.c init_utils.c parse_utils.c parse.c redirection_utils.c
UTILS_FILES = mns_utils.c env_utils.c mns_split.c
EXEC_FILES = exec.c exec_redir.c exec_utils.c exec_utils1.c
CFLAGS = -Wall -Wextra -Werror
CC = gcc
LINKINGS = -Llibft -lft -lreadline
INCLUDE = -I./libft -I./include
SRCS_FILES += $(addprefix src/builtins/, $(BUILTINS_FILES))
SRCS_FILES += $(addprefix src/handle_signal/, $(SIGNAL_FILES))
SRCS_FILES += $(addprefix src/lexer/, $(LEXER_FILES))
SRCS_FILES += $(addprefix src/check_tokens/, $(CHECK_TOKENS_FILES))
SRCS_FILES += $(addprefix src/expander/, $(EXPANDER_FILES))
SRCS_FILES += $(addprefix src/handler_heredoc/, $(HANDLE_HEREDOC_FILES))
SRCS_FILES += $(addprefix src/handle_input/, $(HANDLE_INPUT_FILES))
SRCS_FILES += $(addprefix src/init/, $(INIT_FILES))
SRCS_FILES += $(addprefix src/parser/, $(PARSER_FILES))
SRCS_FILES += $(addprefix src/utils/, $(UTILS_FILES))
SRCS_FILES += $(addprefix src/exec/, $(EXEC_FILES))
LIBFT = ./libft
RM = rm -rf

## RULES
all: $(NAME)

$(NAME): $(SRCS_FILES)
	make -C $(LIBFT) bonus
	$(CC) $(CFLAGS) $(INCLUDE) $(SRCS_FILES) -o $(NAME) $(LINKINGS)

clean:
	make -C $(LIBFT) clean
	$(RM) *.o */*.o */*/*.o

fclean: clean
	make -C $(LIBFT) fclean
	$(RM) $(NAME)

re: fclean all

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --suppressions=readline.donto ./$(NAME)

.PHONY: all clean fclean re
