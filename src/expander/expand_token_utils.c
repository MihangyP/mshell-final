#include <minishell.h>

char	*finalize_result(char *result)
{
	if (result == NULL)
		return (ft_strdup(""));
	return (result);
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
