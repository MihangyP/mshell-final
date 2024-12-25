#include <minishell.h>

int	print_syntax_error(void)
{
	ft_putstr_fd("Syntax error: expected filename after redirection"
		" or expected delimiter after heredoc\n", 2);
	return (0);
}

static int	check_redirection(t_token **current_token)
{
	*current_token = (*current_token)->next;
	if (is_invalid_redirection(*current_token))
		return (print_syntax_error());
	return (1);
}

static int	check_pipe(t_token *current_token, int arg_count)
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
