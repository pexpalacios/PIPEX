/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: penpalac <penpalac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/23 15:49:03 by penpalac          #+#    #+#             */
/*   Updated: 2025/09/23 17:25:24 by penpalac         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

static char	*get_envp(char **envp, char *key)
{
	int		i = 0;
	char	*temp;

	while (envp[i])
	{
		int j = 0;
		while (envp[i][j] && envp[i][j] != '=')
			j++;
		temp = ft_tempstr(envp[i], 0, j);
		if (ft_strcmp(temp, key) == 0)
		{
			free(temp);
			return (envp[i] + j + 1);
		}
		free(temp);
		i++;
	}
	return (NULL);
}

static char	*get_path(char *cmd, char **envp)
{
	char	**paths = ft_split(get_envp(envp, "PATH"), ':');
	char	**cmd_args = ft_split(cmd, ' ');
	char	*full_path = NULL;
	char	*temp;
	int		i = 0;

	if (!paths || !cmd_args)
		return (NULL);
	while (paths[i])
	{
		temp = ft_strjoin(paths[i], "/");
		full_path = ft_strjoin(temp, cmd_args[0]);
		free(temp);
		if (access(full_path, F_OK | X_OK) == 0)
			break;
		free(full_path);
		full_path = NULL;
		i++;
	}
	free_matrix(paths);
	free_matrix(cmd_args);
	return (full_path);
}

static void	exec_cmd(char *cmd, char **envp)
{
	char	**args = ft_split(cmd, ' ');
	char	*path;

	if (!args)
		exit(127);
	if (ft_strchr(args[0], '/') > -1)
		path = args[0];
	else
		path = get_path(args[0], envp);
	if(!path)
	{
		free_matrix(args);
		perror("Error: Command not found");
		exit(127);
	}
	execve(path, args, envp);
	perror("Error: Command not found");
	free_matrix(args);
	exit(127);
}

static void	fork_pid(char *cmd, char **envp, int fdin)
{
	int		fd[2];
	pid_t	pid;

	if (pipe(fd) == -1)
	{
		perror("Error: pipe failed (1)");
		exit(1);
	}
	pid = fork();
	if (pid == -1)
	{
		perror("Error: pid failed");
		exit(1);
	}

	if (pid > 0) //parent
	{
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		close (fd[0]);
	}
	else // child
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		if (fdin < 0)
			exit(1);
		exec_cmd(cmd, envp);
	}
}

static void	heredoc(char *delimiter)
{
	pid_t	pid;
	int		fd[2];
	char	*line;

	if(pipe(fd) == -1)
	{
		perror("Error: pipe failed");
		exit(1);
	}
	pid = fork();
	if (pid == -1)
	{
		perror("Error: fork failed");
		exit(1);
	}

	if (pid)
	{
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		waitpid(pid, NULL, 0);
	}
	else
	{
		close(fd[0]);
		while (1)
		{
			write(1, "here_doc > ", 11);
			line = get_next_line(0);
			if (!line)
				exit(EXIT_SUCCESS);
			if (ft_strncmp(line, delimiter, ft_strlen(delimiter)) == 0)
			{
				free(line);
				exit(EXIT_SUCCESS);
			}
			write(fd[1], line, ft_strlen(line));
			free(line);
		}
	}
}

int	main (int ac, char **av, char **envp)
{
	int		fdin, fdout, i;

	if (ac < 5)
	{
		perror("Error: Invalid number of arguments");
		return (1);
	}
	
	if (ft_strcmp(av[1], "here_doc") == 0)
	{
		heredoc(av[2]);
		fdout = open(av[ac - 1], O_CREAT | O_WRONLY | O_TRUNC, 0777);
		dup2(fdout, 1);
		i = 3;	
	}
	else
	{
		fdin = open(av[1], O_RDONLY);
		fdout = open(av[ac - 1], O_CREAT | O_WRONLY | O_TRUNC, 0777);
		dup2(fdin, 0);
		dup2(fdout, 1);
		i = 2;	
	}

	for (; i < ac - 2; i++)
		fork_pid(av[i], envp, 1);
	exec_cmd(av[i], envp);
	wait(NULL);
	if (ft_strcmp(av[1], "here_doc") == 0)
		unlink("here_doc"); //delete temp
	return (0);
}