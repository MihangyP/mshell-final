#include <minishell.h>

void	set_null_terminators(t_ast_node *cmd, int *count, int *counts)
{
	if (count[ARG_COUNT] != 0)
		cmd->args[counts[ARG_COUNT]] = NULL;
	if (count[REDIR_COUNT] != 0)
		cmd->redirection[counts[REDIR_COUNT]].target = NULL;
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
