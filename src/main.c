#include <minishell.h>

/*volatile sig_atomic_t	g_signal = 0;*/
int	g_signal = 0;

void	free_env_lst(t_list *env_lst)
{
	t_list	*tmp;

	while (env_lst)
	{
		tmp = env_lst;
		env_lst = env_lst->next;
		free(tmp->content);
		free(tmp);
	}
	free(env_lst);
}

void	handler_sigint(int signum)
{
	if (signum == SIGINT)
	{
		set(130);
		write(1, "\n", 1);
		rl_on_new_line();
		rl_replace_line("", 1);
		rl_redisplay();
	}
}

char	*str_append(char *str, char *append)
{
	char	*result;
	int		len1;
	int		len2;

	len1 = 0;
	if (str != NULL)
		len1 = ft_strlen(str);
	len2 = ft_strlen(append);
	result = (char *)malloc(sizeof(char) * (len1 + len2 + 1));
	if (str)
		ft_strlcpy(result, str, (len1 + 1));
	ft_strlcpy(result + len1, append, len2 + 1);
	free(str);
	return (result);
}

size_t  count_word(const char *str, char c)
{
    int         i;
    size_t     words;
    int         in_quotes;
    char        type_quote;

    i = 0;
    type_quote = 0;
	in_quotes = 0;
    words = 0;
    i = 0;
    while (str[i])
    {
        while (str[i] && str[i] == c)
            i++;
        if (str[i])
        {
            words++;
            while (str[i] && (in_quotes || str[i] != c))
            {
                if (str[i] == '\'' || str[i] == '"')
                {
                    if (in_quotes && type_quote == str[i])
                        in_quotes = 0;
                    else if (!in_quotes)
                    {
                        in_quotes = 1;
                        type_quote = str[i];
                    }
                }
                i++;
            }
        }
    }
    return (words);
}

size_t  len_word(const char *str, char c, size_t start)
{
    size_t  len;
    char    type_quote;
    int     in_quotes;

    len = 0;
    in_quotes = 0;
    while (str[start + len] && (in_quotes || str[start + len] != c))
    {
        if (str[start + len] == '\'' || str[start + len] == '"')
        {
            if (in_quotes && type_quote == str[start + len])
                in_quotes = 0;
            else if (!in_quotes)
            {
                in_quotes = 1;
                type_quote = str[start + len];
            }
        }
    	len++;
	}
	return (len);
}

void	fc_split(const char *str, char c, char **split, size_t words)
{
	size_t	i;
	size_t	j;
	size_t	k;

	i = 0;
	j = 0;
	k = 0;
	while (i < words)
	{
		while (str[j] && str[j] == c)
			j++;
		split[i] = (char *)malloc(sizeof(char) * (len_word(str, c, j) + 1));
		while (k < len_word(str, c, j))
		{
			split[i][k] = str[j + k];
			k++;
		}
		split[i][k] = '\0';
		k = 0;
		j += len_word(str, c, j);
		i++;
	}
	split[i] = NULL;
}

char	**split_mns(const char *str, char c)
{
	size_t	words;
	char	**split;

	if (str == NULL)
		return (NULL);
	words = count_word(str, c);
	split = (char **)malloc(sizeof(char *) * (words + 1));
	if (split == NULL)
		return (NULL);
	fc_split(str, c, split, words);
	return (split);
}

void	executor(char **env, t_ast_node *ast, t_exec_status *status)
{
	if (ast->type == 0)
		exec_cmd(env, ast, status);
	else if (ast->type == 1)
		pipe_cmd(env, ast, status);
}

void	wait_pipe_cmd(int fd[2], t_exec_status *status, int pid, int pid1)
{
	close(fd[1]);
	close(fd[0]);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	waitpid(pid, NULL, 0);
	waitpid(pid1, &(status->status), 0);
	if (WIFEXITED(status->status))
		status->status = WEXITSTATUS(status->status);
}

void	pipe_cmd(char **env, t_ast_node *ast, t_exec_status *status)
{
	int			fd[2];
	pid_t		pid;
	pid_t		pid1;

	pid1 = 0;
	if (pipe(fd) == -1)
		return ;
	pid = fork();
	if (pid == 0)
		pipe_exec_left(fd, ast, env, status);
	else
	{
		pid1 = fork();
		if (pid1 == 0)
		{
			status->in_pipe += 1;
			pipe_exec_right(fd, ast, env, status);
		}
	}
	wait_pipe_cmd(fd, status, pid, pid1);
}

int	check_directory_error(char *path, char *cmd)
{
	struct stat		stats;

	stat(path, &stats);
	if (S_ISDIR(stats.st_mode))
	{
		ft_putstr_fd(cmd, 2);
		ft_putendl_fd(" : Is a directory", 2);
		return (1);
	}
	return (0);
}

char	*check_path(char **path_list, char *cmd)
{
	int		i;
	char	*path;
	char	*tmp;

	i = 0;
	if (path_list == NULL)
		return (NULL);
	while (path_list[i])
	{
		tmp = ft_strjoin(path_list[i], "/");
		path = ft_strjoin(tmp, cmd);
		if (access(path, F_OK) == 0)
		{
			free(tmp);
			return (path);
		}
		else
		{
			free(tmp);
			free(path);
			i++;
		}
	}
	return (0);
}

char	*find_path(char **path_list, char *cmd)
{
	if (cmd == 0)
		return (0);
	if (ft_strchr(cmd, '/'))
	{
		if (access(cmd, F_OK) == 0)
			return (cmd);
		return (0);
	}
	return (check_path(path_list, cmd));
}

char	**find_path_list(char **env)
{
	int		i;
	char	*tmp;

	i = 0;
	while (env[i])
	{
		tmp = ft_strnstr(env[i], "PATH", 4);
		if (tmp != 0)
			break ;
		i++;
	}
	if (tmp)
		return (ft_split(tmp + 5, ':'));
	else
		return (NULL);
}

void	execute(t_ast_node *ast, char **env, char **cmd, int *flag)
{
	char			**path_list;
	char			*path;

	(void)ast;
	path_list = find_path_list(env);
	path = find_path(path_list, cmd[0]);
	if (path_list)
		free_split(path_list);
	if (path == NULL)
	{
		ft_putstr_fd(cmd[0], 2);
		if (ft_strchr(cmd[0], '/') == 0)
			ft_putstr_fd(" : command not found\n", 2);
		else
			ft_putstr_fd(" : No such file or directory\n", 2);
		*flag = 127;
		return ;
	}
	if (execve(path, cmd, env) == -1)
	{
		if (check_directory_error(path, cmd[0]) == 0)
			perror(cmd[0]);
		*flag = 126;
		return ;
	}
}

int	check_n_exec_built_in1(char **cmd, char **env, int *flag)
{
	if (ft_strncmp(cmd[0], "export", 7) == 0)
	{
		export_mshell(cmd, &env);
		*flag = 0;
		return (1);
	}
	else if (ft_strncmp(cmd[0], "cd", 3) == 0)
	{
		cd_mshell(cmd, &env);
		*flag = 0;
		return (1);
	}
	else if (ft_strncmp(cmd[0], "unset", 6) == 0)
	{
		unset_mshell(cmd, &env);
		*flag = 0;
		return (1);
	}
	else if (ft_strncmp(cmd[0], "env", 4) == 0)
	{
		env_mshell(env, cmd);
		*flag = 0;
		return (1);
	}
	return (0);
}

int	check_n_exec_built_in(char **cmd, char **env, t_ast_node *ast, int *flag)
{
	if (ft_strncmp(cmd[0], "pwd", 4) == 0)
	{
		pwd_mshell(cmd, env);
		*flag = 0;
		return (1);
	}
	else if (ft_strncmp(cmd[0], "echo", 5) == 0)
	{
		echo_mshell(cmd);
		*flag = 0;
		return (1);
	}
	else if (ft_strncmp(cmd[0], "exit", 5) == 0)
	{
		*flag = exit_mshell(cmd, ast, env, 0);
		return (1);
	}
	return (0);
}

int	redir_input(char *input, t_ast_node *node, char **env, int flag)
{
	int	fd;

	fd = open(input, O_RDONLY);
	if (fd < 0)
	{
		perror (input);
		if (flag == 1)
			return (0);
		free_ast(&node);
		free_split(env);
		exit(EXIT_FAILURE);
	}
	dup2(fd, 0);
	close(fd);
	return (1);
}

int	redir_output(char *output, t_ast_node *node, char **env, int flag)
{
	int	fd;

	fd = open(output, O_RDONLY | O_WRONLY | O_CREAT
			| O_TRUNC, 0644);
	if (fd < 0)
	{
		perror (output);
		if (flag == 1)
			return (0);
		free_ast(&node);
		free_split(env);
		exit (EXIT_FAILURE);
	}
	dup2(fd, 1);
	close(fd);
	return (1);
}

int	output_append(char *out_append, t_ast_node *node, char **env, int flag)
{
	int	fd;

	fd = open(out_append, O_RDONLY | O_WRONLY
			| O_CREAT | O_APPEND, 0644);
	if (fd < 0)
	{
		perror (out_append);
		if (flag == 1)
			return (0);
		free_ast(&node);
		free_split(env);
		exit (EXIT_FAILURE);
	}
	dup2(fd, 1);
	close(fd);
	return (1);
}

int	here_doc(int in_pipe, t_ast_node *node, char **env, int flag)
{
	char	*nb;
	char	*name;
	int		fd;

	nb = ft_itoa(in_pipe);
	name = ft_strjoin(".tmp", nb);
	fd = open(name, O_RDONLY);
	free(name);
	free(nb);
	if (fd < 0)
	{
		perror ("heredoc");
		if (flag == 1)
			return (0);
		free_ast(&node);
		free_split(env);
		exit (EXIT_FAILURE);
	}
	dup2(fd, 0);
	close(fd);
	return (1);
}

int	check_redirection_exec(t_ast_node *ast, char **env, int in_pipe, int flag)
{
	int	i;
	int	status;

	i = 0;
	status = 1;
	while (ast->redirection[i].target)
	{
		if (status == 0)
			return (status);
		if (ast->redirection[i].type_redirection == REDIRECTION_IN)
			status = redir_input(ast->redirection[i].target, ast, env, flag);
		else if (ast->redirection[i].type_redirection == REDIRECTION_OUT)
			status = redir_output(ast->redirection[i].target, ast, env, flag);
		else if (ast->redirection[i].type_redirection == REDIRECTION_APPEND)
			status = output_append(ast->redirection[i].target, ast, env, flag);
		else if (ast->redirection[i].type_redirection == REDIRECTION_HEREDOC)
			status = here_doc(in_pipe, ast, env, flag);
		i++;
	}
	return (status);
}

char	**check_void_cmd(char **cmd, char **env, t_ast_node *ast)
{
	char	**tmp;
	int		i;

	if (cmd == NULL)
		return (NULL);
	i = 0;
	tmp = cmd;
	while (ft_strlen(tmp[i]) == 0 && tmp[i] != NULL)
		i++;
	if (tmp[i] == NULL)
	{
		free_split(env);
		free_ast(&ast);
		exit(EXIT_SUCCESS);
	}
	tmp += i;
	return (tmp);
}

void	exec_cmd(char **env, t_ast_node *ast, t_exec_status *status)
{
	char	**tmp;

	tmp = check_void_cmd(ast->args, env, ast);
	if (ast->redirection)
		check_redirection_exec(ast, env, status->in_pipe, 0);
	if (ast->args == NULL)
		return ;
	if (check_n_exec_built_in(tmp, env, ast, &status->status) == 1
		|| check_n_exec_built_in1(tmp, env, &status->status) == 1)
		return ;
	execute(ast, env, tmp, &status->status);
}

void	pipe_exec_left(int fd[2], t_ast_node *ast, char **env,
					t_exec_status *status)
{
	close(fd[0]);
	dup2(fd[1], 1);
	close(fd[1]);
	executor(env, ast->left, status);
}

int	pipe_exec_right( int fd[2], t_ast_node *ast, char **env,
					t_exec_status *status)
{
	close(fd[1]);
	dup2(fd[0], 0);
	close(fd[0]);
	executor(env, ast->right, status);
	return (status->status);
}

void	uptdate_history(t_mshell *mshell)
{
	char	*path;
	char	*tmp;

	add_history(mshell->entry);
	tmp = ft_strjoin(mshell->path, "/");
	path = ft_strjoin(tmp, ".history_file");
	mshell->hist_fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0777);
	if (mshell->hist_fd != -1)
	{
		ft_putendl_fd(mshell->entry, mshell->hist_fd);
		close(mshell->hist_fd);
	}
	free(path);
	free(tmp);
}

int	check_built_in(t_ast_node *ast)
{
	if (ft_strncmp(ast->args[0], "cd", 3) == 0)
		return (1);
	else if (ft_strncmp(ast->args[0], "export", 7) == 0)
		return (1);
	else if (ft_strncmp(ast->args[0], "unset", 6) == 0)
		return (1);
	else if (ft_strncmp(ast->args[0], "exit", 5) == 0)
		return (1);
	else
		return (0);
}

void	execute_fork_cmd(t_mshell *mshell, char **env, t_ast_node *ast)
{
	pid_t			pid;
	t_exec_status	status;

	status.status = 0;
	status.in_pipe = 0;
	pid = fork();
	if (pid == 0)
	{
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		executor(env, ast, &status);
		free_ast(&ast);
		free_split(env);
		exit(status.status);
	}
	else
		wait_child_process(mshell);
}

void	handle_ast(t_mshell *mshell)
{
	if (mshell->ast->type == AST_COMMAND && mshell->ast->args != NULL
		&& check_built_in(mshell->ast) == 1)
		handle_built_in_cmd(mshell->ast, &mshell->env, mshell);
	else
		execute_fork_cmd(mshell, mshell->env, mshell->ast);
}

void	set_null_terminators(t_ast_node *cmd, int *count, int *counts)
{
	if (count[ARG_COUNT] != 0)
		cmd->args[counts[ARG_COUNT]] = NULL;
	if (count[REDIR_COUNT] != 0)
		cmd->redirection[counts[REDIR_COUNT]].target = NULL;
}

void	handle_redirection(t_token **tokens, t_redirection *redirection,
	int *file_count, int count)
{
	redirection[*file_count].fd = (*tokens)->fd;
	if ((*tokens)->type == TOKEN_REDIR_IN)
		redirection[*file_count].type_redirection = REDIRECTION_IN;
	if ((*tokens)->type == TOKEN_REDIR_OUT)
		redirection[*file_count].type_redirection = REDIRECTION_OUT;
	if ((*tokens)->type == TOKEN_REDIR_APPEND)
		redirection[*file_count].type_redirection = REDIRECTION_APPEND;
	if ((*tokens)->type == TOKEN_HEREDOC)
		redirection[*file_count].type_redirection = REDIRECTION_HEREDOC;
	*tokens = (*tokens)->next;
	if ((*tokens)->type == TOKEN_WORD && count != 0)
	{
		redirection[*file_count].target = ft_strdup((*tokens)->value);
		(*file_count)++;
	}
}

void	process_token(t_token **tokens, t_ast_node *cmd, int *count,
	int *counts)
{
	if ((*tokens)->type == TOKEN_WORD && count[ARG_COUNT] != 0)
	{
		if ((*tokens)->value != NULL)
			cmd->args[counts[ARG_COUNT]++] = ft_strdup((*tokens)->value);
		else
			cmd->args[counts[ARG_COUNT]++] = NULL;
	}
	if ((*tokens)->type == TOKEN_REDIR_IN)
		handle_redirection(tokens, cmd->redirection,
			&counts[REDIR_COUNT], count[REDIR_COUNT]);
	if ((*tokens)->type == TOKEN_REDIR_OUT)
		handle_redirection(tokens, cmd->redirection,
			&counts[REDIR_COUNT], count[REDIR_COUNT]);
	if ((*tokens)->type == TOKEN_REDIR_APPEND)
		handle_redirection(tokens, cmd->redirection,
			&counts[REDIR_COUNT], count[REDIR_COUNT]);
	if ((*tokens)->type == TOKEN_HEREDOC)
		handle_redirection(tokens, cmd->redirection,
			&counts[REDIR_COUNT], count[REDIR_COUNT]);
}

void	count_redirection(t_token **tokens, int *count)
{
	if ((*tokens)->type == TOKEN_REDIR_IN)
	{
		*tokens = (*tokens)->next;
		if ((*tokens)->type == TOKEN_WORD)
			count[REDIR_COUNT] += 1;
	}
	if ((*tokens)->type == TOKEN_REDIR_OUT)
	{
		*tokens = (*tokens)->next;
		if ((*tokens)->type == TOKEN_WORD)
			count[REDIR_COUNT] += 1;
	}
	if ((*tokens)->type == TOKEN_REDIR_APPEND)
	{
		*tokens = (*tokens)->next;
		if ((*tokens)->type == TOKEN_WORD)
			count[REDIR_COUNT] += 1;
	}
	if ((*tokens)->type == TOKEN_HEREDOC)
	{
		*tokens = (*tokens)->next;
		if ((*tokens)->type == TOKEN_WORD)
			count[REDIR_COUNT] += 1;
	}
}

void	count_type_token(t_token *tokens, int *count)
{
	t_token	*tmp;

	tmp = tokens;
	while (tmp != NULL && tmp->type != TOKEN_PIPE
		&& tmp->type != TOKEN_EOF)
	{
		if (tmp->type == TOKEN_WORD)
			count[ARG_COUNT] += 1;
		if (tmp->type == TOKEN_REDIR_IN_OUT)
		{
			tmp = tmp->next;
			if (tmp->type == TOKEN_WORD)
				count[REDIR_COUNT] += 1;
		}
		count_redirection(&tmp, count);
		tmp = tmp->next;
	}
}

t_ast_node	*init_node(t_ast_node_type type)
{
	t_ast_node	*node;

	node = (t_ast_node *)malloc(sizeof(t_ast_node));
	node->type = type;
	node->args = NULL;
	node->left = NULL;
	node->right = NULL;
	node->redirection = NULL;
	return (node);
}

void	init_args_input_output_file(t_ast_node **cmd, int *count)
{
	if (count[ARG_COUNT] != 0)
		(*cmd)->args = (char **)ft_calloc(count[ARG_COUNT] + 1,
				sizeof(char *));
	if (count[REDIR_COUNT] != 0)
		(*cmd)->redirection = (t_redirection *)ft_calloc(count[REDIR_COUNT] + 1,
				sizeof(t_redirection));
}

t_ast_node	*parse_token(t_token **tokens, t_ast_node *cmd)
{
	int	counts[3];
	int	count[3];
	int	i;

	i = 0;
	while (i < 3)
	{
		counts[i] = 0;
		count[i] = 0;
		i++;
	}
	count_type_token(*tokens, count);
	init_args_input_output_file(&cmd, count);
	while (tokens != NULL && (*tokens)->type != TOKEN_PIPE
		&& (*tokens)->type != TOKEN_EOF)
	{
		process_token(tokens, cmd, count, counts);
		*tokens = (*tokens)->next;
	}
	set_null_terminators(cmd, count, counts);
	return (cmd);
}

t_ast_node	*parse_command(t_token **tokens)
{
	t_ast_node	*node;

	node = init_node(AST_COMMAND);
	node = parse_token(tokens, node);
	return (node);
}

t_ast_node	*parse_pipeline(t_token **tokens)
{
	t_ast_node	*left;
	t_ast_node	*pipe;

	left = parse_command(tokens);
	if (*tokens != NULL && (*tokens)->type == TOKEN_PIPE)
	{
		pipe = init_node(AST_PIPE);
		pipe->left = left;
		*tokens = (*tokens)->next;
		pipe->right = parse_pipeline(tokens);
		return (pipe);
	}
	return (left);
}

t_ast_node	*parse(t_token *tokens)
{
	t_ast_node	*root;

	root = parse_pipeline(&tokens);
	return (root);
}

void	process_token_split(t_token **token, t_token **last,
	t_token *new_token)
{
	if (*token == NULL)
	{
		*token = new_token;
		*last = new_token;
	}
	else
	{
		(*last)->next = new_token;
		*last = new_token;
	}
}

int	is_ignored_dollar(char current, char next,
	int in_double_quotes, int in_single_quotes)
{
	if (current == '$' && (next == '\'' || next == '"')
		&& (in_double_quotes || in_single_quotes))
		return (1);
	if (current == '$' && (next == '\'' || next == '"'))
		return (0);
	return (1);
}

char	*remove_quotes(char *str)
{
	int		in_d_quotes;
	int		in_s_quotes;
	char	*result;
	int		i;

	result = NULL;
	i = 0;
	in_d_quotes = 0;
	in_s_quotes = 0;
	while (str[i] != '\0')
	{
		if (handle_quotes_expander(str[i], &in_s_quotes, &in_d_quotes))
		{
			
		}
		else
		{
			if (is_ignored_dollar(str[i], str[i + 1], in_d_quotes,
				in_s_quotes) == 1)
				result = char_append(result, str[i]);
		}
		i++;
	}
	return (result);
}

t_token	*split_word(char *word)
{
	char	*word_without_quotes;
	char	**split;
	int		i;
	t_token	*token;
	t_token	*last;

	token = NULL;
	last = NULL;
	i = 0;
	split = split_mns(word, ' ');
	if (split[i] == NULL)
		return (free_split(split), create_token(TOKEN_WORD, "", &i, -1));
	while (split[i] != NULL)
	{
		word_without_quotes = remove_quotes(split[i]);
		process_token_split(&token, &last,
			create_token(TOKEN_WORD, word_without_quotes, NULL, -1));
		free(word_without_quotes);
		i++;
	}
	free_split(split);
	return (token);
}

char	*finalize_result(char *result)
{
	if (result == NULL)
		return (ft_strdup(""));
	return (result);
}

int	should_expand(char *str, int i, int in_single_quotes)
{
	return (str[i] == '$' && !in_single_quotes
		&& (ft_isalnum(str[i + 1]) || str[i + 1] == '_' || str[i + 1] == '?'));
}

int	handle_quotes_expander(char c, int *in_single_quotes, int *in_double_quotes)
{
	if (c == '\'' && !(*in_double_quotes))
	{
		*in_single_quotes = !(*in_single_quotes);
		return (1);
	}
	else if (c == '"' && !(*in_single_quotes))
	{
		*in_double_quotes = !(*in_double_quotes);
		return (1);
	}
	return (0);
}

void	init_expand_vars(t_expand_result *result, int *i)
{
	*i = -1;
	result->value = NULL;
	result->create_token = 0;
	result->prev = 0;
}

void	init_expand_context(t_expand_context *ctx,
			t_expand_result *result, int exit_code)
{
	ctx->result = result;
	ctx->exit_code = exit_code;
	ctx->in_single_quotes = 0;
	ctx->in_double_quotes = 0;
}

void	expand_char(t_expand_context *ctx)
{
	char	*tmp;

	tmp = expand_special_char(ctx->str, ctx->env, ctx->i, ctx->exit_code);
	ctx->result->value = str_append(ctx->result->value, tmp);
	if (!ctx->in_double_quotes && ctx->result->prev != '=')
		ctx->result->create_token = 1;
	free(tmp);
}

void	handle_non_expand_char(t_expand_context *ctx, char *str, int i)
{
	if (is_ignored_dollar(str[i], str[i + 1], ctx->in_double_quotes,
			ctx->in_single_quotes) == 1)
		ctx->result->value = char_append(ctx->result->value, str[i]);
}

t_expand_result	expand_token(char *str, char **env, int exit_code)
{
	t_expand_result		result;
	t_expand_context	ctx;
	int					i;

	init_expand_vars(&result, &i);
	init_expand_context(&ctx, &result, exit_code);
	ctx.str = str;
	ctx.env = env;
	ctx.i = &i;
	while (str[++i])
	{
		if (handle_quotes_expander(str[i], &ctx.in_single_quotes,
				&ctx.in_double_quotes) == 1)
			handle_non_expand_char(&ctx, str, i);
		else if (should_expand(str, i, ctx.in_single_quotes))
			expand_char(&ctx);
		else
			handle_non_expand_char(&ctx, str, i);
		ctx.result->prev = str[i];
	}
	result.value = finalize_result(result.value);
	return (result);
}

t_token	*process_word(t_token *current, t_expand_params *params)
{
	char			*word_without_quotes;
	t_token			*new_token;
	int				tmp;
	t_expand_result	expanded;

	expanded.value = NULL;
	expanded = expand_token(current->value, params->env, params->exit_code);
	if (expanded.create_token == 1)
		new_token = split_word(expanded.value);
	else
	{
		word_without_quotes = remove_quotes(expanded.value);
		new_token = create_token(TOKEN_WORD, word_without_quotes, &tmp, -1);
		free(word_without_quotes);
	}
	params->expanded_tokens = add_new_token(params->expanded_tokens,
			new_token, params->last);
	while (new_token && new_token->next)
		new_token = new_token->next;
	*(params->last) = new_token;
	free(expanded.value);
	return (params->expanded_tokens);
}

t_token	*add_new_token(t_token *expanded_tokens, t_token *new_token,
	t_token **last)
{
	if (expanded_tokens == NULL)
		expanded_tokens = new_token;
	else
		(*last)->next = new_token;
	*last = new_token;
	return (expanded_tokens);
}

t_token	*process_heredoc(t_token *current, t_token *expanded_tokens,
	t_token **last)
{
	t_token	*new_token;
	int		tmp;

	new_token = create_token(current->type, current->value, &tmp, -1);
	expanded_tokens = add_new_token(expanded_tokens, new_token, last);
	return (expanded_tokens);
}

int	process_skip(t_token *current, t_expand_params *params, int flag)
{
	params->expanded_tokens = process_heredoc(current, params->expanded_tokens,
			params->last);
	if (flag == 1)
		return (1);
	return (0);
}

void	init_expand_params(t_expand_params *params, char **env,
	int exit_code, t_token **last)
{
	params->env = env;
	params->exit_code = exit_code;
	params->expanded_tokens = NULL;
	params->last = last;
}

t_token	*expand_tokens(t_token *tokens, char **env, int exit_code)
{
	t_token			*current;
	t_token			*last;
	int				skip_next;
	t_expand_params	params;

	current = tokens;
	last = NULL;
	skip_next = 0;
	init_expand_params(&params, env, exit_code, &last);
	while (current)
	{
		if (skip_next)
			skip_next = process_skip(current, &params, 0);
		else if (current->type == TOKEN_HEREDOC)
			skip_next = process_skip(current, &params, 1);
		else if (current->type == TOKEN_WORD)
			params.expanded_tokens = process_word(current, &params);
		else
			params.expanded_tokens = process_heredoc(current,
					params.expanded_tokens, params.last);
		current = current->next;
	}
	return (params.expanded_tokens);
}

void	free_token(t_token *token)
{
	t_token	*tmp;

	if (token == NULL)
		return ;
	while (token != NULL)
	{
		tmp = token;
		token = token->next;
		free(tmp->value);
		free(tmp);
	}
	token = NULL;
}

int	check_pipe(t_token *current_token, int arg_count)
{
	if (current_token->next == NULL || arg_count == 0
		|| current_token->next->type == TOKEN_EOF)
	{
		ft_putstr_fd("Syntax error: expected command after pipe.\n"
			, 2);
		return (0);
	}
	return (1);
}

int	print_syntax_error(void)
{
	ft_putstr_fd("Syntax error: expected filename after redirection"
		" or expected delimiter after heredoc\n", 2);
	return (0);
}

int	check_redirection(t_token **current_token)
{
	*current_token = (*current_token)->next;
	if (is_invalid_redirection(*current_token))
		return (print_syntax_error());
	return (1);
}

int	is_redirection(t_tokentype type)
{
	if (type == TOKEN_HEREDOC || type == TOKEN_REDIR_APPEND
		|| type == TOKEN_REDIR_IN || type == TOKEN_REDIR_OUT
		|| type == TOKEN_REDIR_IN_OUT)
		return (1);
	return (0);
}

int	analyze_tokens(t_token *tokens)
{
	t_token	*current;
	int		arg_count;

	current = tokens;
	arg_count = 0;
	while (current != NULL && current->type != TOKEN_EOF)
	{
		if (current->type == TOKEN_WORD)
			arg_count++;
		if (is_redirection(current->type))
		{
			if (check_redirection(&current) == 0)
				return (0);
			arg_count++;
		}
		if (current->type == TOKEN_PIPE)
		{
			if (check_pipe(current, arg_count) == 0)
				return (0);
			arg_count = 0;
		}
		current = current->next;
	}
	return (1);
}

int	is_delimiter(const char *line, const char *heredoc_delimiter)
{
	if (line == NULL)
		return (1);
	if (ft_strncmp(line, heredoc_delimiter,
			ft_strlen(heredoc_delimiter) + 1) == 0)
		return (1);
	return (0);
}

int	check_delimiter(char *heredoc_delimiter, char *result, char *line)
{
	if (is_delimiter(heredoc_delimiter, result))
	{
		free(line);
		if (result != NULL)
			free(result);
		return (1);
	}
	return (0);
}

char	*get_env_value(char *var, char **env)
{
	int		i;
	int		var_len;

	i = 0;
	var_len = ft_strlen(var);
	while (env[i])
	{
		if (ft_strncmp(env[i], var, var_len) == 0
			&& env[i][var_len] == '=')
			return (env[i] + var_len + 1);
		i++;
	}
	return (NULL);
}

int	get_var_name_length(char *str)
{
	int		i;

	i = 0;
	if (ft_isdigit(str[i]))
		return (1);
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	return (i);
}

char	*expand_env_var(char *var_name, char **env)
{
	int		var_len;
	char	*var;
	char	*value;

	var_len = get_var_name_length(var_name);
	var = (char *)malloc(sizeof(char) * (var_len + 1));
	ft_strlcpy(var, var_name, (var_len + 1));
	value = get_env_value(var, env);
	free(var);
	if (value == NULL)
		return (NULL);
	return (value);
}

char	*expand_special_char(char *str, char **env, int *i, int exit_code)
{
	char	*result;
	char	*var_value;

	result = NULL;
	var_value = NULL;
	if (str[*i + 1] == '?')
	{
		result = ft_itoa(exit_code);
		(*i)++;
	}
	else
	{
		var_value = expand_env_var(str + *i + 1, env);
		result = (char *)malloc(sizeof(char) * (ft_strlen(var_value) + 3));
		result[0] = '"';
		ft_strlcpy(result + 1, var_value, ft_strlen(var_value) + 1);
		result[ft_strlen(var_value) + 1] = '"';
		printf("result: %s\n", result);
		*i += get_var_name_length(str + *i + 1);
	}
	return (result);
}

char	*special_expand_char(char *str, char **env, int exit_code)
{
	char	*result;
	char	*tmp;
	int		i;

	i = -1;
	tmp = NULL;
	result = NULL;
	while (str[++i])
	{
		if (str[i] == '$'
			&& (ft_isalnum(str[i + 1]) || str[i + 1] == '_'
				|| str[i + 1] == '?'))
		{
			tmp = expand_special_char(str, env, &i, exit_code);
			result = str_append(result, tmp);
		free(tmp);
		}
		else
			result = char_append(result, str[i]);
	}
	return (result);
}

char	*get_result(char *line, int is_expand, char **env, int exit_code)
{
	char	*result;

	result = NULL;
	if (line != NULL && is_expand == 0)
		result = special_expand_char(line, env, exit_code);
	else
		result = ft_strdup(line);
	return (result);
}

int	init_heredoc(char **line, int fd)
{
	*line = readline("> ");
	if (*line == NULL)
	{
		if (g_signal)
		{
			close(fd);
			return (-1);
		}
		close(fd);
		return (-2);
	}
	return (1);
}

int	process_heredoc_redir(t_utils_heredoc utils_var, char **env,
	int exit_code)
{
	char	*line;
	char	*result;

	line = NULL;
	result = NULL;
	while (1 && utils_var.fd > 0)
	{
		if (init_heredoc(&line, utils_var.fd) < 0)
		{
			if (g_signal == SIGINT)
				return (130);
			return (166);
		}
		result = get_result(line, utils_var.is_expand, env, exit_code);
		if (check_delimiter(utils_var.tmp_value, result, line))
			break ;
		ft_putendl_fd(result, utils_var.fd);
		free(line);
		free(result);
	}
	close(utils_var.fd);
	return (1);
}

char	*get_name_file(int in_pipe)
{
	char	*nb;
	char	*name;

	nb = NULL;
	nb = ft_itoa(in_pipe);
	name = ft_strjoin(".tmp", nb);
	free(nb);
	return (name);
}

int	mns_strcmp(char *s1, char *s2)
{
	int	i;

	i = 0;
	while (s1[i] != '\0' || s2[i] != '\0')
	{
		if (s1[i] != s2[i])
			return (s1[i] - s2[i]);
		i++;
	}
	return (0);
}

int	determine_expansion(t_token *current_token, char *tmp)
{
	if (mns_strcmp(current_token->value, tmp) == 0)
		return (0);
	return (1);
}

char	*char_append(char *str, char c)
{
	char	*result;
	int		len;

	len = 0;
	if (str != NULL)
		len = ft_strlen(str);
	result = (char *)malloc(sizeof(char) * (len + 2));
	if (str)
		ft_strlcpy(result, str, (len + 1));
	result[len] = c;
	result[len + 1] = '\0';
	free(str);
	return (result);
}

char	*expand_for_heredoc(char *heredoc_delimiter)
{
	char	*result;
	int		i;

	result = NULL;
	i = 0;
	while (heredoc_delimiter[i] != '\0')
	{
		if (heredoc_delimiter[i] == '\'' || heredoc_delimiter[i] == '"')
			i++;
		else if (heredoc_delimiter[i] == '$' && (heredoc_delimiter[i + 1]
				== '\'' || heredoc_delimiter[i + 1] == '"'))
			i++;
		else
		{
			result = char_append(result, heredoc_delimiter[i]);
			i++;
		}
	}
	if (result == NULL)
		return (ft_strdup(""));
	return (result);
}

int	is_invalid_redirection(t_token *token)
{
	return (token == NULL || token->type != TOKEN_WORD);
}

int	process_heredoc_token(t_token **current, t_mshell *mshell, int exit_code,
	int in_pipe)
{
	char			*name;
	int				status;
	t_utils_heredoc	utils_heredoc;

	name = NULL;
	utils_heredoc.tmp_value = NULL;
	*current = (*current)->next;
	if (is_invalid_redirection(*current))
		return (-2);
	utils_heredoc.tmp_value = expand_for_heredoc((*current)->value);
	utils_heredoc.is_expand = determine_expansion(*current,
			utils_heredoc.tmp_value);
	name = get_name_file(in_pipe);
	utils_heredoc.fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	status = process_heredoc_redir(utils_heredoc, mshell->env, exit_code);
	free(name);
	free(utils_heredoc.tmp_value);
	return (status);
}

int	process_tokens_heredoc(t_token *token, t_mshell *mshell, int exit_code)
{
	static int	in_pipe;
	int			status;
	t_token		*tmp_token;

	in_pipe = 0;
	status = 0;
	tmp_token = token;
	while (tmp_token)
	{
		if (tmp_token->type == TOKEN_PIPE)
			in_pipe += 1;
		if (tmp_token->type == TOKEN_HEREDOC)
			status = process_heredoc_token(&tmp_token, mshell, exit_code,
					in_pipe);
		if (status == -2)
			break ;
		tmp_token = tmp_token->next;
	}
	return (status);
}

void	block_if_for_lexer(t_token **current, t_token **head
	, t_token *new_token)
{
	if (*head == NULL)
	{
		*head = new_token;
		*current = new_token;
	}
	else
	{
		(*current)->next = new_token;
		*current = new_token;
	}
}

void	block_if_for_get_len_word(char *input, int index,
	int	*in_quotes, char *type_quotes)
{
	if (!*in_quotes)
	{
		*in_quotes = 1;
		*type_quotes = input[index];
	}
	else if (input[index] == *type_quotes)
	{
		*in_quotes = 0;
		*type_quotes = 0;
	}
}

int	is_special_char(char c)
{
	if (c == '|' || c == '<' || c == '>' || c == ' ')
		return (1);
	return (0);
}

int	get_len_word(char *input, int *index)
{
	int		start;
	int		end;
	int		in_quotes;
	char	type_quotes;

	in_quotes = 0;
	type_quotes = 0;
	start = *index;
	end = start;
	while (input[end] != '\0')
	{
		if (!in_quotes && is_special_char(input[end]) == 1)
			break ;
		if (input[end] == '\'' || input[end] == '"')
			block_if_for_get_len_word(input, end, &in_quotes, &type_quotes);
		end++;
	}
	return (end - start + 1);
}

t_token	*get_token_word(char *input, int *index)
{
	t_token	*token;
	char	*word;
	int		len;

	len = get_len_word(input, index);
	word = malloc(sizeof(char) * len);
	ft_strlcpy(word, input + *index, len);
	if (word == NULL)
		return (NULL);
	token = create_token(TOKEN_WORD, word, index, -1);
	free(word);
	return (token);
}

t_token	*create_token(t_tokentype type, const char *value, int *index,
	int fd)
{
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (token == NULL)
		return (NULL);
	token->type = type;
	token->value = ft_strdup(value);
	if (fd == 1 || fd == 2 || fd == 0)
		token->fd = fd;
	else
		token->fd = 0;
	token->next = NULL;
	if (index != NULL)
		*index += ft_strlen(value);
	return (token);
}

int	get_fd(char *input, int *index)
{
	int	fd;

	fd = -1;
	if (ft_isdigit(input[*index]) && (input[*index + 1] == '>'
			|| input[*index + 1] == '<'))
	{
		fd = ft_atoi(input + *index);
		*index += 1;
	}
	return (fd);
}

t_token	*get_next_token(char *input, int *index)
{
	int	fd;

	fd = get_fd(input, index);
	if (input[*index] == '|')
		return (create_token(TOKEN_PIPE, "|", index, fd));
	else if (input[*index] == '>')
	{
		if (input[*index + 1] == '>')
			return (create_token(TOKEN_REDIR_APPEND, ">>", index, fd));
		return (create_token(TOKEN_REDIR_OUT, ">", index, fd));
	}
	else if (input[*index] == '<')
	{
		if (input[*index + 1] == '>')
			return (create_token(TOKEN_REDIR_IN_OUT, ">", index, fd));
		if (input[*index + 1] == '<')
			return (create_token(TOKEN_HEREDOC, "<<", index, fd));
		return (create_token(TOKEN_REDIR_IN, "<", index, fd));
	}
	else if (input[*index] == '\n')
		return (create_token(TOKEN_NEWLINE, "\n", index, fd));
	else if (input[*index] == '\0')
		return (create_token(TOKEN_EOF, "\0", index, fd));
	else
		return (get_token_word(input, index));
}

int	mns_is_space(char c)
{
	if ((c >= 9 && c <= 13) || c == ' ')
		return (1);
	return (0);
}

t_token	*lexer(char *input)
{
	t_token	*head;
	t_token	*current;
	t_token	*new_token;
	int		i;

	head = NULL;
	current = NULL;
	i = 0;
	while (i < (int)ft_strlen(input))
	{
		if (mns_is_space(input[i]))
		{
			i++;
			continue ;
		}
		new_token = get_next_token(input, &i);
		if (new_token == NULL)
			break ;
		block_if_for_lexer(&current, &head, new_token);
	}
	if (current == NULL)
		return (NULL);
	current->next = create_token(TOKEN_EOF, "\0", &i, -1);
	return (head);
}

void	handler_sigint_heredoc(int sig)
{
	(void)sig;
	g_signal = sig;
	write(1, "\n", 1);
	close(STDIN_FILENO);
}

void	setup_signals_heredoc(struct sigaction *sa)
{
	sigemptyset(&sa->sa_mask);
	sa->sa_flags = 0;
	sa->sa_handler = handler_sigint_heredoc;
	sigaction(SIGINT, sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}

void	handle_child_process(t_mshell *mshell, int exit_code)
{
	struct sigaction	sa;
	t_token				*token;
	int					status;

	g_signal = 0;
	setup_signals_heredoc(&sa);
	status = 1;
	token = lexer(mshell->entry);
	if (token == NULL)
		exit(status);
	status = process_tokens_heredoc(token, mshell, exit_code);
	free_token(token);
	free_split(mshell->env);
	free(mshell->entry);
	exit(status);
}

int	wait_process(t_mshell *mshell)
{
	int	status;

	sigaction(SIGINT, &mshell->sa_ignore, NULL);
	wait(&status);
	sigaction(SIGINT, &mshell->sa, NULL);
	if (WIFEXITED(status))
	{
		if (WEXITSTATUS(status) == 130)
			return (130);
		else
			return (WEXITSTATUS(status));
	}
	return (0);
}

int	heredoc(t_mshell *mshell, int exit_code)
{
	pid_t	pid_child;

	pid_child = fork();
	if (pid_child == -1)
	{
		perror("fork");
		return (-1);
	}
	else if (pid_child == 0)
		handle_child_process(mshell, exit_code);
	else
		return (wait_process(mshell));
	return (0);
}

int	handle_value(int value)
{
	static int	val;

	if (value == -69)
		return (val);
	val = value;
	return (value);
}

void	set(int value)
{
	handle_value(value);
}

int	get(void)
{
	return (handle_value(-69));
}

int	check_eof(char *str)
{
	if (*str == '\0')
	{
		free(str);
		return (1);
	}
	return (0);
}

int	handle_state(int state)
{
	if (state == 130)
	{
		close_tmp();
		return (1);
	}
	else if (state == 166)
	{
		close_tmp();
		return (2);
	}
	return (0);
}

int	line_is_space(char *line)
{
	int	i;

	i = 0;
	while (line[i] == ' ')
		i++;
	if (line[i] == '\0')
		return (1);
	return (0);
}

int	process_input(t_mshell *mshell)
{
	int	state;

	uptdate_history(mshell);
	if (*mshell->entry == '\0')
		return (0);
	if (line_is_space(mshell->entry))
		return (0);
	state = heredoc(mshell, mshell->exit_code);
	if (handle_state(state) == 1)
		return (1);
	else if (handle_state(state) == 2)
		return (2);
	if (check_eof(mshell->entry) == 1)
		return (1);
	if (get() == 130)
			mshell->exit_code = 130;
	process_line(mshell);
	return (0);
}

void	close_tmp(void)
{
	char	*name;
	char	*nb;
	int		i;

	i = 0;
	while (1)
	{
		nb = ft_itoa(i);
		name = ft_strjoin(".tmp", nb);
		if (unlink(name) == -1)
		{
			free(nb);
			free(name);
			break ;
		}
		free(nb);
		free(name);
		i++;
	}
}

void	free_redirection(t_redirection *redirection)
{
	int	i;

	i = 0;
	while (redirection[i].target != NULL)
	{
		free(redirection[i].target);
		i++;
	}
	free(redirection);
}

void	free_split(char **str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		free(str[i]);
		i++;
	}
	free(str);
}

void	free_ast(t_ast_node **node)
{
	if (node == NULL || *node == NULL)
		return ;
	if ((*node)->type != AST_PIPE && (*node)->args != NULL)
		free_split((*node)->args);
	if ((*node)->type != AST_PIPE && (*node)->redirection != NULL)
		free_redirection((*node)->redirection);
	if ((*node)->left != NULL)
		free_ast(&(*node)->left);
	if ((*node)->right != NULL)
		free_ast(&(*node)->right);
	free(*node);
}

void	free_mshell(t_mshell *mshell)
{
	free_ast(&mshell->ast);
	close_tmp();
	free(mshell->entry);
	mshell->ast = NULL;
}

void	get_history(int fd)
{
	char	*line;

	if (fd <= 0)
		return ;
	while (1)
	{
		line = get_next_line(fd);
		if (line != NULL)
		{
			line[ft_strlen(line) - 1] = '\0';
			add_history(line);
			free(line);
		}
		else
			break ;
	}
}

void	setup_signals(t_mshell *mshell)
{
	sigemptyset(&mshell->sa_sigquit.sa_mask);
	sigemptyset(&mshell->sa.sa_mask);
	sigemptyset(&mshell->sa_ignore.sa_mask);
	mshell->sa.sa_handler = handler_sigint;
	mshell->sa.sa_flags = 0;
	sigaction(SIGINT, &mshell->sa, NULL);
	mshell->sa_ignore.sa_handler = SIG_IGN;
	mshell->sa_sigquit.sa_flags = 0;
	mshell->sa_ignore.sa_flags = 0;
	mshell->sa_sigquit.sa_handler = SIG_IGN;
	sigaction(SIGQUIT, &mshell->sa_sigquit, NULL);
}

void	init_mshell(t_mshell *mshell, char **env)
{
	mshell->entry = NULL;
	mshell->env = cpy_env(env);
	mshell->token = NULL;
	mshell->ast = NULL;
	mshell->fd_tmp = -1;
	mshell->exit_code = 0;
	getcwd(mshell->path, PATH_MAX);
	mshell->hist_fd = open(".history_file", O_RDONLY, 0777);
	if (mshell->hist_fd > 0)
	{
		get_history(mshell->hist_fd);
		close(mshell->hist_fd);
	}
	setup_signals(mshell);
}

void	handle_built_in_cmd(t_ast_node *ast, char ***env, t_mshell *mshell)
{
	int		fd_in;
	int		fd_out;

	fd_in = dup(STDIN_FILENO);
	fd_out = dup(STDOUT_FILENO);
	if (ast->redirection && check_redirection_exec(ast, *env, 0, 1) == 0)
	{
		mshell->exit_code = 1;
		mns_close_fds(fd_in, fd_out);
		return ;
	}
	if (ft_strncmp(ast->args[0], "cd", 3) == 0)
		mshell->exit_code = cd_mshell(ast->args, env);
	else if (ft_strncmp(ast->args[0], "export", 7) == 0)
		mshell->exit_code = export_mshell(ast->args, env);
	else if (ft_strncmp(ast->args[0], "unset", 6) == 0)
		mshell->exit_code = unset_mshell(ast->args, env);
	else if (ft_strncmp(ast->args[0], "exit", 5) == 0)
	{
		mns_close_fds(fd_in, fd_out);
		mshell->exit_code = exit_mshell(ast->args, ast, *env, 1);
	}
	dup2(fd_in, STDIN_FILENO);
	dup2(fd_out, STDOUT_FILENO);
	mns_close_fds(fd_in, fd_out);
}

void	wait_child_process(t_mshell *mshell)
{
	int	status;

	sigaction(SIGINT, &mshell->sa_ignore, NULL);
	wait(&status);
	sigaction(SIGINT, &mshell->sa, NULL);
	if (WIFEXITED(status))
		mshell->exit_code = WEXITSTATUS(status);
	if (WIFSIGNALED(status))
	{
		write(1, "\n", 1);
		mshell->exit_code = 128 + WTERMSIG(status);
	}
}

void	process_line(t_mshell *mshell)
{
	t_token		*expanded;

	mshell->token = lexer(mshell->entry);
	if (mshell->token == NULL)
	{
		mshell->ast = NULL;
		return ;
	}
	if (analyze_tokens(mshell->token) == 0)
	{
		close_tmp();
		free_token(mshell->token);
		mshell->exit_code = 2;
		return ;
	}
	expanded = expand_tokens(mshell->token, mshell->env, mshell->exit_code);
	free_token(mshell->token);
	mshell->ast = parse(expanded);
	free_token(expanded);
	if (mshell->ast->type == AST_COMMAND && mshell->ast->args != NULL
		&& mshell->ast->args[0] == NULL)
		return ;
	handle_ast(mshell);
}

bool	repl(t_mshell *mshell)
{
	int	status;

	while (true)
	{
		set(0);
		mshell->entry = readline("mshell> ");
		if (!mshell->entry)
			return (false);
		status = process_input(mshell);
		if (status == 1)
		{
			mshell->exit_code = 130;
			continue ;
		}
		else if (status == 2)
			continue ;
		free_mshell(mshell);
	}
	return (true);
}

int	main(int ac, char **av, char **env)
{
	t_mshell	mshell;

	(void)ac;
	(void)av;
	init_mshell(&mshell, env);
	if (!repl(&mshell))
	{
		write(1, "exit\n", 5);
		free_mshell(&mshell);
		free_split(mshell.env);
	}
	return (0);
}
