/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: penpalac <penpalac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/23 15:49:03 by penpalac          #+#    #+#             */
/*   Updated: 2025/09/24 19:09:13 by penpalac         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Replicates the way pipes work in bash.
// Usage: ./pipex fdin cmd1 cmd2 cmd3 fdout >>>>> fdin < cmd1 | cmd2 | cmd3 > fdout

#include "pipex.h"

char	**get_cmds(int start, char **av);
static	void	heredoc(char *delimiter);
void	run_pipeline(char **cmds, int n, const char *infile, const char *outfile, char **envp);
static char	*get_path(char *cmd, char **envp);
static char	*get_envp(char **envp, char *key);

int main(int ac, char **av, char **envp) 
{
	char **cmds;
	int cmd_count;
	char *infile, *outfile;

	if (ac < 5)
	{
		perror("Error: Invalid number or arguments");
		return (1);
	}

	//check heredoc
	if (ft_strncmp(av[1], "here_doc", 8) == 0)
	{
		if (ac < 6)
		{
			ft_putstr_fd("Error: heredoc needs at least 6 arguments\n", 2);
			return (1);
		}
		heredoc(av[2]);
		infile = "here_doc";
		cmd_count = ac - 4;
		cmds = get_cmds(3, av);
		if (!cmds)
		{
			unlink("here_doc");
			return (1);
		}
		if (access("here_doc", R_OK) != 0)
		{
			ft_putstr_fd("Error: heredoc file not accessible\n", 2);
			return (1);
		}
	}
	else
	{
		infile = av[1];
		cmd_count = ac - 3;
		cmds = get_cmds(2, av);
	}
	outfile = av[ac - 1];
	run_pipeline(cmds, cmd_count, infile, outfile, envp);

	free(cmds);
	if (ft_strncmp(av[1], "here_doc", 8) == 0)
		unlink("here_doc");
	return (0);
}

// Auxiliar function that cuts the arguments to make a command array
char	**get_cmds(int start, char **av)
{
	char	**cmds = malloc(12 * sizeof(char *));
	int i = 0;

	while(av[start])
	{
		cmds[i] = av[start];
		i++;
		start++;
	}
	return (cmds);
}

// Auxiliar that creates and reads the here_doc file
static void	heredoc(char *delimiter)
{
	size_t	delim_len = ft_strlen(delimiter);
	char	*line;

	int fd = open("here_doc", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
	{
		perror("heredoc");
		exit(1);
	}

	while (1)
	{
		write(1, "heredoc> ", 9);
		line = get_next_line(0);
		if (!line)
		{
			close(fd);
			exit(1);
		}

		if (ft_strlen(line) > 0)
		{
			if (line[ft_strlen(line) - 1] == '\n')
            	line[ft_strlen(line) - 1] = '\0';

			if (ft_strncmp(line, delimiter, delim_len) == 0)
			{
				free(line);
				break;
			}
			write(fd, line, ft_strlen(line));
			free(line);
		}
	}
	close(fd);
	fd = open("here_doc", O_RDONLY);
	if (fd < 0)
	{
		perror("heredoc");
		exit(1);
	}
	close(fd);
}

// cmds: array of command strings, n: number of commands, infile/outfile: filenames
void run_pipeline(char **cmds, int n, const char *infile, const char *outfile, char **envp) 
{
	pid_t *pids = malloc(n * sizeof(pid_t));
	int prev_fd = -1;
	int fd[2];
	char **argv;
	char *cmd_path;

	for (int i = 0; i < n; i++) 
	{
		if ((i < (n - 1)) && pipe(fd) == -1) 
		{
			perror("pipe");
			exit(1);
		}

		pids[i] = fork();
		if (pids[i] == 0) 
		{
			// First command: set input from infile
			if (i == 0 && infile) 
			{
				int in = open(infile, O_RDONLY);
				if (in < 0) 
				{ 
					perror("open infile"); exit(1); 
				}
				dup2(in, STDIN_FILENO);
				close(in);
			}
			// Last command: set output to outfile
			if ((i == (n - 1)) && outfile) 
			{
				int out = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
				if (out < 0) 
				{ 
					perror("open outfile"); 
					exit(1); 
				}
				dup2(out, STDOUT_FILENO);
				close(out);
			}
			// Middle commands: set input from prev_fd
			if (prev_fd != -1) 
			{
				dup2(prev_fd, STDIN_FILENO);
				close(prev_fd);
			}
			// Not last: set output to pipe
			if (i < n - 1) 
			{
				printf("HERE!");
				close(fd[0]);
				dup2(fd[1], STDOUT_FILENO);
				close(fd[1]);
			}
			argv = ft_split(cmds[i], ' ');
			cmd_path = get_path(argv[0], envp);
			execve(cmd_path, argv, envp);
			//execve failed
			free(cmd_path);
			free_matrix(argv);
			perror("execve");
			exit(127);
		}
		// Parent
		if (prev_fd != -1)
			close(prev_fd);
		if (i < n - 1) {
			close(fd[1]);
			prev_fd = fd[0];
		}
	}
	// Wait for all children
	for (int i = 0; i < n; i++)
		waitpid(pids[i], NULL, 0);
	free(pids);
}

// Auxiliar function for exec_cmd. Gets the path for the commands
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

// Auxiliar for get_path. Gets the variable PATH from the environment
static char	*get_envp(char **envp, char *key)
{
	int		i = 0;
	char	*temp;

	while (envp[i])
	{
		int j = 0;
		while (envp[i][j] && envp[i][j] != '=')
			j++;
		temp = ft_substr(envp[i], 0, j);
		if (ft_strncmp(temp, key, ft_strlen(key)) == 0)
		{
			free(temp);
			return (envp[i] + j + 1);
		}
		free(temp);
		i++;
	}
	return (NULL);
}