#include <minishell.h>

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

int	process_input(t_data *data)
{
	int	state;

	uptdate_history(data);
	if (*data->line == '\0')
		return (0);
	if (line_is_space(data->line))
		return (0);
	state = heredoc(data, data->exit_status);
	if (handle_state(state) == 1)
		return (1);
	else if (handle_state(state) == 2)
		return (2);
	if (check_eof(data->line) == 1)
		return (1);
	if (get_variable() == 130)
			data->exit_status = 130;
	process_line(data);
	return (0);
}

void	cleanup_data(t_data *data)
{
	free_ast(&data->ast);
	close_tmp();
	free(data->line);
	data->ast = NULL;
}
