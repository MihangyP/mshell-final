#include <minishell.h>

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
