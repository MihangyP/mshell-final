#include <minishell.h>

int	ft_env(char **env, char **cmd)
{
	int	i;

	if (cmd[1])
	{
		ft_putstr_fd("Env error\n", 2);
		return (EXIT_FAILURE);
	}
	i = 0;
	while (env[i])
	{
		ft_putendl_fd(env[i], 1);
		i ++;
	}
	return (EXIT_SUCCESS);
}
