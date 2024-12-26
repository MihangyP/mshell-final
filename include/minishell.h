#ifndef MINISHELL_H
# define MINISHELL_H

# include <libft.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <stdio.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <signal.h>
# include <stdbool.h>
# include <limits.h>

// Defines 

# define REDIR_COUNT 0
# define ARG_COUNT 1

// Enums 

typedef enum	e_ast_node_type
{
	AST_COMMAND,
	AST_PIPE,
}	t_ast_node_type;

typedef enum	e_type_redirection
{
	REDIRECTION_IN,
	REDIRECTION_OUT,
	REDIRECTION_APPEND,
	REDIRECTION_HEREDOC,
}	t_type_redirection;

typedef enum e_tokentype
{
	TOKEN_WORD,
	TOKEN_PIPE,
	TOKEN_REDIR_IN,
	TOKEN_REDIR_OUT,
	TOKEN_REDIR_IN_OUT,
	TOKEN_REDIR_APPEND,
	TOKEN_HEREDOC,
	TOKEN_NEWLINE,
	TOKEN_EOF
}	t_tokentype;

// Structures

typedef struct	s_exec_status
{
	int			status;
	int			in_pipe;
}	t_exec_status;


typedef struct	s_token
{
	t_tokentype		type;
	char			*value;
	int				fd;
	struct s_token	*next;
}	t_token;

typedef struct	s_redirection
{
	t_type_redirection		type_redirection;
	int						fd;
	char					*target;
}	t_redirection;

typedef struct	s_ast_node
{
	t_ast_node_type		type;
	char				**args;
	t_redirection		*redirection;
	struct s_ast_node	*left;
	struct s_ast_node	*right;
}	t_ast_node;

typedef struct s_mshell
{
	char				*entry;
	char				**env;
	t_token				*token;
	t_ast_node			*ast;
	int					hist_fd;
	struct sigaction	sa;
	struct sigaction	sa_sigquit;
	struct sigaction	sa_ignore;
	int					exit_code;
	int					fd_tmp;
	char				path[PATH_MAX];
}	t_mshell;

typedef struct s_expand_params
{
	char		**env;
	int			exit_code;
	t_token		*expanded_tokens;
	t_token		**last;
}				t_expand_params;

typedef struct s_expand_result
{
	char	*value;
	char	prev;
	int		create_token;
}	t_expand_result;

typedef struct s_expand_context
{
	t_expand_result	*result;
	char			*str;
	char			**env;
	int				*i;
	int				exit_code;
	int				in_single_quotes;
	int				in_double_quotes;
}	t_expand_context;

typedef struct s_utils_heredoc
{
	char	*tmp_value;
	int		is_expand;
	int		fd;
}	t_utils_heredoc;

// Prototypes

void	free_token(t_token *token);
int		is_valid_var_char(char c);
int		valid_name_assignement(const char *value);
int		mns_is_space(char c);
int		is_special_char(char c);
int		get_fd(char *input, int *index);

t_token	*create_token(t_tokentype type, const char *value, int *index,
			int fd);
t_token	*get_next_token(char *input, int *index);
t_token	*lexer(char *input);

int	cd_mshell(char **cmd, char ***env);
int	pwd_mshell(char **args, char **env);
int	echo_mshell(char **args);
int	env_mshell(char **env, char **cmd);
int	export_mshell(char **cmd, char ***env);
int	unset_mshell(char **cmd, char ***env);
int	exit_mshell(char **cmd, t_ast_node *ast, char **env, int flag);
int		check_redirection_exec(t_ast_node *ast, char **env,
			int in_pipe, int flag);
char	**find_path_list(char **env);
char	*find_path(char **path_list, char *cmd);
void	free_split(char **str);
void	exec_cmd(char **env, t_ast_node *ast, t_exec_status *status);
void	executor(char **env, t_ast_node *ast, t_exec_status *status);
int		check_n_exec_built_in1(char **cmd, char **env, int *flag);
int		check_n_exec_built_in(char **cmd, char **env,
			t_ast_node *ast, int *flag);
char	**check_void_cmd(char **cmd, char **env, t_ast_node *ast);
void	execute(t_ast_node *ast, char **env, char **cmd, int *flag);
int		check_directory_error(char *path, char *cmd);
void	wait_pipe_cmd(int fd[2], t_exec_status *status, int pid, int pid1);
void	handle_built_in_cmd(t_ast_node *ast, char ***env, t_mshell *mshell);
void	handle_ast(t_mshell *mshell);
char	**list_to_tab(t_list *env_lst);
void	sort_env_lst(t_list	**env_lst);
t_list	*get_env_lst(char **env);
t_list	*get_env_lst(char **env);
char	**list_to_tab(t_list *env_lst);
void	print_export(t_list *env_lst);
void	print_var_value(char *value, int i, t_list *env_lst);
int		check_var_name(char *name);
char	*get_var_name(char *var);
char	*get_var_value(char *var);
char	**cpy_env(char **env);
int		mns_strcmp(char *s1, char *s2);
t_list	*get_env_lst(char **env);
void	free_env_lst(t_list *env_lst);
int		lst_srch_var(t_list *lst, char *var_name);
void	mns_close_fds(int fd1, int fd2);

int				get_var_name_length(char *str);
char			*get_env_value(char *var, char **env);
char			*expand_for_heredoc(char *heredoc_delimiter);
char			*str_append(char *str, char *append);
char			*char_append(char *str, char c);
char			*expand_env_var(char *var_name, char **env);
void			handle_quotes(char c, int *in_single_quotes,
					int *in_double_quotes);
char			*expand_special_char(char *str, char **env, int *i,
					int exit_code);
int				should_expand(char *str, int i, int in_single_quotes);
t_token			*add_new_token(t_token *expanded_tokens, t_token *new_token,
					t_token **last);
t_token			*process_word(t_token *current, t_expand_params *params);
char			*finalize_result(char *result);
int				is_ignored_dollar(char current, char next,
					int in_double_quotes, int in_single_quotes);
int				handle_quotes_expander(char c, int *in_single_quotes,
					int *in_double_quotes);
t_expand_result	expand_token(char *str, char **env, int exit_code);
t_token			*expand_tokens(t_token *tokens, char **env, int exit_code);

void		free_ast(t_ast_node **node);
t_ast_node	*init_node(t_ast_node_type type);
void		init_args_input_output_file(t_ast_node **cmd, int *count);

void		set_null_terminators(t_ast_node *cmd, int *count, int *counts);
void		process_token(t_token **tokens, t_ast_node *cmd, int *count,
				int *counts);

void		count_redirection(t_token **tokens, int *count);
void		count_type_token(t_token *tokens, int *count);
void		handle_redirection(t_token **tokens, t_redirection *redirection,
				int *file_count, int count);
t_ast_node	*parse(t_token *tokens);

int	is_redirection(t_tokentype type);
int	is_delimiter(const char *line, const char *heredoc_delimiter);
int	is_invalid_redirection(t_token *token);
int	determine_expansion(t_token *current_token, char *tmp);

int	handle_heredoc(t_token **current_token, char **env, int exit_code);

int	print_syntax_error(void);
int	analyze_tokens(t_token *tokens);

void	handler_sigint_heredoc(int sig);

char	*get_name_file(int in_pipe);
int		process_heredoc_redir(t_utils_heredoc utils_var, char **env,
			int exit_code);
int		wait_process(t_mshell *mshell);
char	*expand_for_heredoc(char *heredoc_delimiter);
char	*special_expand_char(char *str, char **env, int exit_code);
int		process_tokens_heredoc(t_token *token, t_mshell *mshell, int exit_code);
void	setup_signals_heredoc(struct sigaction *sa);
int		process_heredoc_token(t_token **current, t_mshell *mshell,
			int exit_code, int in_pipe);
char	*get_result(char *line, int is_expand, char **env, int exit_code);
int		check_delimiter(char *heredoc_delimiter, char *result, char *line);
int		heredoc(t_mshell *mshell, int exit_code);

int		line_is_space(char *line);
int		handle_exit(t_mshell *mshell);
void	close_tmp(void);
void	process_line(t_mshell *mshell);
int		handle_state(int state);
int		process_input(t_mshell *mshell);
void	free_mshell(t_mshell *mshell);
char	*expand_line(char *line, char **env, int exit_code);
void	handler_sigint(int signum);
int		check_eof(char *str);
void	uptdate_history(t_mshell *mshell);
int		check_built_in(t_ast_node *ast);
void	wait_child_process(t_mshell *mshell);
void	execute_fork_cmd(t_mshell *mshell, char **env, t_ast_node *ast);
void	init_mshell(t_mshell *mshell, char **env);
void	remove_one(t_list **env_lst, char *to_remove);	
char	*get_var_value(char *var);

int		handle_value(int value);
void	set(int value);
int		get(void);
char	*remove_quotes(char *str);
char	**split_mns(const char *str, char c);
void	pipe_exec_left(int fd[2], t_ast_node *ast, char **env, t_exec_status *status);
int	pipe_exec_right( int fd[2], t_ast_node *ast, char **env, t_exec_status *status);
void	pipe_cmd(char **env, t_ast_node *ast, t_exec_status *status);

#endif
