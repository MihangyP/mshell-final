#include <minishell.h>

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
