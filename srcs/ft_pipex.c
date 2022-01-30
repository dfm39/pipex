/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pipex.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dfranke <dfranke@student.42wolfsburg.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/01/28 13:37:20 by dfranke           #+#    #+#             */
/*   Updated: 2022/01/30 20:00:57 by dfranke          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

void	close_ends(t_env *env, t_cmd *node)
{
	int	j;

	j = 0;
	if (!env->lmt->islmt)
		node->idx--;
	while (j < env->pipe_no)
	{
		if (node->idx == 0 || j != node->idx - 1)
		{
			if (close(env->pipes[j][0]))
				perror("close [X][0]");
		}
		if (j != node->idx || node == env->lst_cmd)
		{
			if (close(env->pipes[j][1]))
				perror("close [X][1]");
		}
		j++;
	}
}

/*	LMT WRITE  NEEDS				[0][1]
	1. CMD NEEDS		[0][0]	&&	[1][1]	||				[0][1]	--> close [0][0]	[1][0] [1][1]
	2. CMD NEEDS		[1][0]	&&	[2][1]	||	[0][0]	&&	[1][1]	--> close [0][1]	[1][0]
	Lst CMD NEEDS		[2][0]				||	[1][0]				--> close [0][0]	[0][1] [1][1]
	*/

void	exec_c(t_env *env, t_cmd *node)
{
	int		i;

	if (dup2(node->s_in, STDIN_FILENO) < 0)
		perror("1st dup");
	if (dup2(node->s_out, STDOUT_FILENO) < 0)
		perror("2nd dup");
	close_ends(env, node);
	i = 0;
	if (execve(node->pth, node->array, env->envp_c))
		perror("execve: ");
}

void	fork_loop(t_env *env)
{
	t_cmd	*node;

	node = env->fir_cmd;
	while (node)
	{
		node->pid = fork();
		if (node->pid < 0)
			perror("cmd fork: ");
		else if (!node->pid)
			break ;
		node = node->next;
	}
	if (node)
		exec_c(env, node);
}

void	pipex(t_env *env)
{
	int		status;
	int		i;

	i = 0;
	while (i < env->pipe_no)
	{	
		if (pipe(env->pipes[i++]) < 0)
			perror("pipe: ");
	}
	set_io(env);
	fork_lmt(env);
	if (env->lmt->pid)
		fork_loop(env);
	i = 0;
	while (i < env->pipe_no)
	{
		close(env->pipes[i][0]);
		close(env->pipes[i++][1]);
	}
	i = -1;
	while (++i < env->cmd_no)
		waitpid(-1, &status, 0);
}
