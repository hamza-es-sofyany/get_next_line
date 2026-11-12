/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hes-sofy <hes-sofy@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 17:26:29 by hes-sofy          #+#    #+#             */
/*   Updated: 2026/11/12 15:35:18 by hes-sofy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"
#include "get_next_line.h"
#include <fcntl.h>
#include <stdio.h>


// static int fill_stash(int fd, char **stash)
// {
    
// }

// char *get_next_line(int fd)
// {
//     static char *stash;
//     char    *str;
//     char    buffer[BUFFER_SIZE + 1];
//     ssize_t byt;

//     if (fd < 0 || BUFFER_SIZE <= 0 || read(fd, NULL, 0) < 0)
//         return (NULL);
//     byt = 1;
//     while (byt > 0)
//     {
//         byt = read(fd, buffer, BUFFER_SIZE);
//         if (byt > 0)
//         {
//             if (byt > 0)
//                 buffer[byt] = '\0';
//             str = append_buffer(stash, buffer);
//             if (!str)
//                 return (NULL);
//              stash = str;
//              free(str);
//         }
//         extract_line(stash);
//         clean_stash(stash);
//     }
//     return (stash);
// }




#include "get_next_line.h"

static int	fill_stash(int fd, char **stash)
{
	char	*buffer;
	char	*new_stash;
	ssize_t	bytes_read;
	size_t	i;

	buffer = malloc(BUFFER_SIZE + 1);
	if (!buffer)
		return (-1);
	i = 0;
	while ((*stash)[i] && (*stash)[i] != '\n')
		i++;
	while (!(*stash)[i])
	{
		bytes_read = read(fd, buffer, BUFFER_SIZE);
		if (bytes_read < 0)
			return (free(buffer), -1);
		if (bytes_read == 0)
			break ;
		buffer[bytes_read] = '\0';
		new_stash = append_buffer(*stash, buffer);
		if (!new_stash)
			return (free(buffer), -1);
		free(*stash);
		*stash = new_stash;
		i = 0;
		while ((*stash)[i] && (*stash)[i] != '\n')
			i++;
	}
	free(buffer);
	return (0);
}

char	*get_next_line(int fd)
{
	static char	*stash;
	char		*line;
	char		*remaining;

	if (fd < 0 || BUFFER_SIZE <= 0 || read(fd, NULL, 0) < 0)
		return (NULL);
	if (fill_stash(fd, &stash) < 0)
		return (NULL);
	if (!stash || !stash[0])
		return (free(stash), stash = NULL, NULL);
	line = extract_line(stash);
	if (!line)
		return (NULL);
	remaining = clean_stash(stash);
	free(stash);
	stash = remaining;
	return (line);
}

int main(void)
{
    int     fd;
    char    *line;

    fd = open("sample.txt", O_RDONLY);
    if (fd < 0)
        return (1);
    while ((line = get_next_line(fd)) != NULL)
    {
        printf("%s", line);
        free(line);
    }
    close(fd);
    return (0);
}
