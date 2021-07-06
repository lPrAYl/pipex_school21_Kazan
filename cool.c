#include "includes/pipex.h"

char	*ft_strjoin_sds(char const *s1, char delimeter, char const *s2)
{
	size_t		i;
	size_t		j;
	char		*dest;

	if (!s1 || !s2)
		return (NULL);
	dest = (char *)malloc(sizeof(char) * (ft_strlen(s1) + ft_strlen(s2) + 2));
	if (!dest)
		return (NULL);
	i = 0;
	while (i < ft_strlen(s1))
	{
		dest[i] = s1[i];
		i++;
	}
	dest[i] = delimeter;
	i++;
	j = 0;
	while (j < ft_strlen(s2))
	{
		dest[i + j] = s2[j];
		j++;
	}
	dest[i + j] = '\0';
	return (dest);
}

char	**find_and_separation_path(char **envp)
{
	int		line;
	char	**chunk;

	line = 0;
	while (envp[line])
	{
		if(ft_strncmp("PATH=", envp[line], 5) == 0)
			break;
		++line;
	}
	chunk = ft_split(envp[line] + 5, ':');
	return (chunk);
}

int	do_execve(char **argv, char **envp, t_pipex *fdp, int i)
{
	int		j;
	char	**cmd;
	char	*path;

	j = 0;
	cmd = ft_split(argv[i + 2], ' ');
	while (fdp->chunk[j])
	{
		path = ft_strjoin_sds(fdp->chunk[j], '/', cmd[0]);
		execve(path, cmd, envp);
		++j;
		free(path);
	}
	perror("Couldn't not find program to execute");
	exit(EXIT_FAILURE);
}


int	main(int argc, char **argv, char **env)
{
	int		i;
	int		num_cmd;
	int		infile;
	int		outfile;
	pid_t	pid;
	t_pipex	*fdp;
	
	if(argc < 5)
	{
		ft_putstr_fd("Not enough arguments!", 1);
		exit(EXIT_FAILURE);
	}
	i = 0;
	num_cmd = argc - 3;
	fdp = (t_pipex *)malloc(sizeof(t_pipex) * (num_cmd - 1)); // количество пайпов
	fdp->chunk = find_and_separation_path(env);
	if(argc == 6 && !ft_strncmp(argv[1], "here_doc", 8))
	{		
		if(!ft_strncmp(argv[1], "here_doc", 8))
		{
			if(pipe(fdp[i].io) == -1)
			{
				perror("Error with creating pipe");
				exit(EXIT_FAILURE);
			}
			pid = fork();
			if(pid == -1)
			{
				perror("Error with creating proccess");
				exit(EXIT_FAILURE);
			}
			if(!pid)
			{
				char	*buf;
				buf = "";
				close(fdp[i].io[0]);
				while(ft_strncmp(buf, argv[2], ft_strlen(argv[2])))
				{
					ft_putstr_fd("heredoc> ", 1);
					if(get_next_line(0, &buf) && ft_strncmp(buf, argv[2], ft_strlen(argv[2])))
					{
						write(fdp[i].io[1], buf, ft_strlen(buf));
						write(fdp[i].io[1], "\n", 1);
					}
				}
				exit(EXIT_SUCCESS);
			}
			i++;
		}
	}
	else
	{
		infile = open(argv[1], O_RDONLY);
		if(infile == -1)
		{
			perror(argv[1]);
			exit(EXIT_FAILURE);
		}
	}
	outfile = open(argv[argc -1], O_RDWR | O_CREAT | O_TRUNC, 0777);
	if(outfile == -1)
	{
		perror(argv[argc - 1]);
		exit(EXIT_FAILURE);
	}
	if(pipe(fdp[i].io) == -1)
	{
		perror("Error with creating pipe");
		exit(EXIT_FAILURE);
	}
	pid = fork();
	if(pid == -1)
	{
		perror("Error with creating proccess");
		exit(EXIT_FAILURE);
	}
	if(!pid)
	{
		if(!ft_strncmp(argv[1], "here_doc", 8) && argc == 6)
		{
			close(fdp[i - 1].io[1]);
			if(dup2(fdp[i - 1].io[0], 0) == -1) // stdin
			{
				perror("Couldn't read from the pipe");
				exit(EXIT_FAILURE);
			}
			close(fdp[i].io[0]);
			if(dup2(fdp[i].io[1], 1) == -1)
			{
				perror("Couldn't write to the pipe");
				exit(EXIT_FAILURE);
			}
			do_execve(argv, env, fdp, i);
		}	
		else
		{
			if(dup2(infile, 0) == -1)
			{
				perror("Couldn't read from the file");
				exit(EXIT_FAILURE);
			}
			close(fdp[i].io[0]);
			if(dup2(fdp[i].io[1], 1) == -1)
			{
				perror("Couldn't write to the pipe");
				exit(EXIT_FAILURE);
			}
			do_execve(argv, env, fdp, i);
		}
	}
	if(pid)
	{
		wait(NULL);
		if(!ft_strncmp(argv[1], "here_doc", 8))
			close(fdp[i - 1].io[1]); // для here_doc -1
		close(fdp[i].io[1]);
		while(i < num_cmd - 2)
		{
			i++;
			if(pipe(fdp[i].io) == -1)
			{
				perror("Error with creating pipe");
				exit(EXIT_FAILURE);
			}
			pid = fork();
			if(pid == -1)
			{
				perror("Error with creating proccess");
				exit(EXIT_FAILURE);
			}
			if(!pid)
			{
				if(dup2(fdp[i - 1].io[0], 0) == -1)
				{
					perror("Couldn't read from the pipe");
					exit(EXIT_FAILURE);
				}
				close(fdp[i].io[0]);
				if(dup2(fdp[i].io[1], 1) == -1)
				{
					perror("Couldn't write to the pipe");
					exit(EXIT_FAILURE);
				}
				do_execve(argv, env, fdp, i);
			}
			if(pid)
			{
				wait(NULL);
				if(!ft_strncmp(argv[1], "here_doc", 8))
					close(fdp[i - 1].io[1]); // для here_doc -1
				close(fdp[i].io[1]);
			}
		}
	}
	close(fdp[i].io[1]);
	if(dup2(fdp[i].io[0], 0) == -1)
	{
		perror("Couldn't read from the pipe");
		exit(EXIT_FAILURE);
	}
	if(dup2(outfile, 1) == -1)
	{
		perror("Couldn't write to the file");
		exit(EXIT_FAILURE);
	}
	do_execve(argv, env, fdp, i + 1);
	printf("!");
	return (0);
}