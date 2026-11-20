/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hes-sofy <hes-sofy>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 17:26:29 by hes-sofy          #+#    #+#             */
/*   Updated: 2026/11/20 10:21:15 by hes-sofy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

static int	ft_has_newline(const char *s)
{
	size_t	i;

	if (!s)
		return (0);
	i = 0;
	while (s[i] && s[i] != '\n')
		i++;
	if (s[i] == '\n')
		return (1);
	return (0);
}

static char	*ft_join_and_free(char *stash, char *buf)
{
	char	*tmp;

	tmp = append_buffer(stash, buf);
	free(stash);
	return (tmp);
}

static char	*ft_free_null(char *s1, char *s2)
{
	free(s1);
	free(s2);
	return (NULL);
}

static char	*ft_read_stash(int fd, char *stash)
{
	ssize_t	byt;
	char	*buf;

	if (ft_has_newline(stash))
		return (stash);
	buf = malloc((size_t)BUFFER_SIZE + 1);
	if (!buf)
		return (ft_free_null(stash, NULL));
	byt = 1;
	while (byt > 0)
	{
		byt = read(fd, buf, BUFFER_SIZE);
		if (byt < 0)
			return (ft_free_null(stash, buf));
		if (byt == 0)
			break ;
		buf[byt] = '\0';
		stash = ft_join_and_free(stash, buf);
		if (!stash)
			break ;
		if (ft_has_newline(buf))
			break ;
	}
	free(buf);
	return (stash);
}

char	*get_next_line(int fd)
{
	static char	*str;
	char		*line;
	char		*tmp;
	int			error;

	if (fd < 0 || BUFFER_SIZE <= 0)
	{
		str = ft_free_null(str, NULL);
		return (NULL);
	}
	str = ft_read_stash(fd, str);
	line = extract_line(str);
	if (!str || !str[0] || !line)
	{
		str = ft_free_null(str, line);
		return (NULL);
	}
	tmp = clean_stash(str, &error);
	free(str);
	str = tmp;
	if (error)
		return (ft_free_null(line, NULL));
	return (line);
}
