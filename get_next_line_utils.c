/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hes-sofy <hes-sofy@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 17:24:16 by hes-sofy          #+#    #+#             */
/*   Updated: 2026/11/13 17:41:31 by hes-sofy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

size_t	ft_strlen(const char *s)
{
	size_t	i;

	if (!s)
		return (0);
	i = 0;
	while (s[i])
		i++;
	return (i);
}

static char	*ft_strjoin(char const *s1, char const *s2)
{
	size_t	i;
	size_t	j;
	size_t	to_len;
	char	*str;

	if (!s1 || !s2)
		return (NULL);
	to_len = ft_strlen(s1) + ft_strlen(s2);
	str = malloc(to_len + 1);
	if (!str)
		return (NULL);
	i = 0;
	while (s1[i])
	{
		str[i] = s1[i];
		i++;
	}
	j = 0;
	while (s2[j])
	{
		str[i + j] = s2[j];
		j++;
	}
	str[i + j] = '\0';
	return (str);
}

char	*append_buffer(char *stash, char *buf)
{
	char	*str;
	size_t	i;

	i = 0;
	if (!buf)
		return (stash);
	if (!stash)
	{
		i++;
		stash = malloc(1);
		if (!stash)
			return (NULL);
		stash[0] = '\0';
	}
	str = ft_strjoin(stash, buf);
	if (i > 0)
		free(stash);
	return (str);
}

char	*extract_line(char *stash)
{
	char	*str;
	size_t	i;

	if (!stash)
		return (NULL);
	i = 0;
	while (stash[i] && stash[i] != '\n')
		i++;
	if (stash[i] == '\n')
		str = malloc(i + 2);
	else
		str = malloc(i + 1);
	if (!str)
		return (NULL);
	i = 0;
	while (stash[i] && stash[i] != '\n')
	{
		str[i] = stash[i];
		i++;
	}
	if (stash[i] == '\n')
		str[i++] = '\n';
	str[i] = '\0';
	return (str);
}

char	*clean_stash(char *stash, int *error)
{
	size_t	i;
	size_t	j;
	char	*str;

	*error = 0;
	if (!stash)
		return (NULL);
	i = 0;
	while (stash[i] && stash[i] != '\n')
		i++;
	if (!stash[i])
		return (NULL);
	i++;
	str = malloc(ft_strlen(&stash[i]) + 1);
	if (!str)
	{
		*error = 1;
		return (NULL);
	}
	j = 0;
	while (stash[i])
		str[j++] = stash[i++];
	str[j] = '\0';
	return (str);
}
