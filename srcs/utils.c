#include "../includes/pipex.h"

void	ft_perror(char *str)
{
	perror(str);
	ft_free_all();
	exit(EXIT_FAILURE);
}

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
	i = -1;
	while (++i < ft_strlen(s1))
		dest[i] = s1[i];
	dest[i++] = delimeter;
	j = -1;
	while (++j < ft_strlen(s2))
		dest[i + j] = s2[j];
	dest[i + j] = '\0';
	return (dest);
}
