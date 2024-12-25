#include <minishell.h>

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
