#include <minishell.h>

int	handle_value(int value)
{
	static int	val;

	if (value == -1)
		return (val);
	val = value;
	return (value);
}

void	set_variable(int value)
{
	handle_value(value);
}

int	get_variable(void)
{
	return (handle_value(-1));
}
