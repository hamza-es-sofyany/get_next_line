#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 42
#endif

#if BUFFER_SIZE <= 0
# error "BUFFER_SIZE must be greater than 0 for this test suite"
#endif

char			*get_next_line(int fd);

typedef struct s_stats
{
	int			total;
	int			passed;
	int			failed;
}				t_stats;

typedef struct s_case
{
	const char	*name;
	const char	*content;
}				t_case;

static void	print_visible(const char *s, size_t len)
{
	size_t			i;
	unsigned char	c;

	i = 0;
	while (i < len)
	{
		c = (unsigned char)s[i];
		if (c == '\n')
			printf("\\n");
		else if (c == '\r')
			printf("\\r");
		else if (c == '\t')
			printf("\\t");
		else if (c == '\\')
			printf("\\\\");
		else if (c >= 32 && c <= 126)
			putchar(c);
		else
			printf("\\x%02X", c);
		i++;
	}
}

static int	write_all(int fd, const char *data, size_t len)
{
	ssize_t	written;
	size_t	total;

	total = 0;
	while (total < len)
	{
		written = write(fd, data + total, len - total);
		if (written < 0)
		{
			if (errno == EINTR)
				continue ;
			return (0);
		}
		if (written == 0)
			return (0);
		total += (size_t)written;
	}
	return (1);
}

static int	create_read_file(const char *data, size_t len)
{
	char	path[] = "/tmp/gnl_suite_XXXXXX";
	int		temp_fd;
	int		read_fd;

	temp_fd = mkstemp(path);
	if (temp_fd < 0)
		return (-1);
	if (!write_all(temp_fd, data, len))
	{
		close(temp_fd);
		unlink(path);
		return (-1);
	}
	if (close(temp_fd) < 0)
	{
		unlink(path);
		return (-1);
	}
	read_fd = open(path, O_RDONLY);
	unlink(path);
	return (read_fd);
}

static int	check_fd_content(int fd, const char *name, const char *expected,
		size_t expected_len)
{
	char	*line;
	size_t	pos;
	size_t	start;
	size_t	line_len;
	size_t	actual_len;
	int		ok;
	int		eof_checks;
	size_t	line_number;

	pos = 0;
	line_number = 0;
	ok = 1;
	while (pos < expected_len)
	{
		start = pos;
		while (pos < expected_len && expected[pos] != '\n')
			pos++;
		if (pos < expected_len && expected[pos] == '\n')
			pos++;
		line_len = pos - start;
		line_number++;
		line = get_next_line(fd);
		if (line == NULL)
		{
			printf("\n    [FAIL] %s: returned NULL before line %zu\n", name,
				line_number);
			printf("           expected: ");
			print_visible(expected + start, line_len);
			printf("\n");
			ok = 0;
			continue ;
		}
		actual_len = strlen(line);
		if (actual_len != line_len || memcmp(line, expected + start,
				line_len) != 0)
		{
			printf("\n    [FAIL] %s: wrong line at byte %zu\n", name, start);
			printf("           expected (%zu): ", line_len);
			print_visible(expected + start, line_len);
			printf("\n           actual   (%zu): ", actual_len);
			print_visible(line, actual_len);
			printf("\n");
			ok = 0;
		}
		free(line);
	}
	eof_checks = 0;
	while (eof_checks < 2)
	{
		line = get_next_line(fd);
		if (line != NULL)
		{
			printf("\n    [FAIL] %s: expected NULL after EOF, got: ", name);
			print_visible(line, strlen(line));
			printf("\n");
			free(line);
			ok = 0;
		}
		eof_checks++;
	}
	return (ok);
}

static void	run_case_data(const char *name, const char *content, size_t len,
		t_stats *stats)
{
	int	fd;
	int	ok;

	stats->total++;
	fd = create_read_file(content, len);
	if (fd < 0)
	{
		printf("[FAIL] %-38s could not create temporary file\n", name);
		stats->failed++;
		return ;
	}
	ok = check_fd_content(fd, name, content, len);
	close(fd);
	if (ok)
	{
		stats->passed++;
		printf("[PASS] %s\n", name);
	}
	else
		stats->failed++;
}

static void	run_case(const char *name, const char *content, t_stats *stats)
{
	run_case_data(name, content, strlen(content), stats);
}

static int	append_format(char *dst, size_t capacity, size_t *used,
		const char *format, ...)
{
	va_list	args;
	int		written;

	if (*used >= capacity)
		return (0);
	va_start(args, format);
	written = vsnprintf(dst + *used, capacity - *used, format, args);
	va_end(args);
	if (written < 0 || (size_t)written >= capacity - *used)
		return (0);
	*used += (size_t)written;
	return (1);
}

static int	append_repeat(char *dst, size_t capacity, size_t *used, char c,
		size_t count)
{
	if (*used >= capacity || count > capacity - *used - 1)
		return (0);
	memset(dst + *used, c, count);
	*used += count;
	dst[*used] = '\0';
	return (1);
}

static void	run_generated_cases(t_stats *stats)
{
	char	content[4096];
	char	name[64];
	size_t	used;
	size_t	i;
	size_t	j;
	size_t	line_count;
	int		kind;
	int		valid;

	i = 0;
	while (i < 100)
	{
		used = 0;
		content[0] = '\0';
		valid = append_format(content, sizeof(content), &used,
				"GENERATED-%03zu|", i);
		line_count = 1 + (i % 9);
		j = 0;
		while (j < line_count && valid)
		{
			kind = (int)((i * 7 + j * 3) % 7);
			if (kind == 0)
				valid = append_format(content, sizeof(content), &used,
						"row-%03zu-%02zu\n", i, j);
			else if (kind == 1)
				valid = append_format(content, sizeof(content), &used, "\n");
			else if (kind == 2)
				valid = append_format(content, sizeof(content), &used,
						" \tvalue-%03zu  \n", i);
			else if (kind == 3)
			{
				valid = append_format(content, sizeof(content), &used,
						"repeat-%03zu:", i);
				if (valid)
					valid = append_repeat(content, sizeof(content), &used,
							(char)('a' + (i + j) % 26), 1 + (i * 13 + j * 7)
							% 90);
				if (valid)
					valid = append_format(content, sizeof(content), &used,
							"\n");
			}
			else if (kind == 4)
				valid = append_format(content, sizeof(content), &used,
						"punct-%03zu: !@#$%%^&*()_+-=[]{};:,.?/\n", i);
			else if (kind == 5)
				valid = append_format(content, sizeof(content), &used,
						"\t\tTAB-%03zu-%02zu\n", i, j);
			else
				valid = append_format(content, sizeof(content), &used,
						"mixed-%03zu-%02zu: 1234567890\n", i, j);
			j++;
		}
		if (!valid)
		{
			printf("[FAIL] generated case %zu: test-data overflow\n", i);
			stats->total++;
			stats->failed++;
			i++;
			continue ;
		}
		if (i % 3 == 1 && used > 0 && content[used - 1] == '\n')
		{
			used--;
			content[used] = '\0';
		}
		snprintf(name, sizeof(name), "Generated pattern %03zu", i + 1);
		run_case_data(name, content, used, stats);
		i++;
	}
}

static void	run_pattern_case(const char *name, size_t line_len,
		int end_with_newline, t_stats *stats)
{
	const char	tail[] = "tail-line\nlast-line";
	char		*content;
	size_t		total;
	size_t		i;

	total = line_len + (size_t)end_with_newline + sizeof(tail) - 1;
	content = (char *)malloc(total + 1);
	if (content == NULL)
	{
		stats->total++;
		stats->failed++;
		printf("[FAIL] %s: test-data malloc failed\n", name);
		return ;
	}
	i = 0;
	while (i < line_len)
	{
		content[i] = (char)('A' + (i % 26));
		i++;
	}
	if (end_with_newline)
		content[i++] = '\n';
	memcpy(content + i, tail, sizeof(tail) - 1);
	i += sizeof(tail) - 1;
	content[i] = '\0';
	run_case_data(name, content, i, stats);
	free(content);
}

static void	run_long_line_cases(t_stats *stats)
{
	static const size_t	lengths[] = {1, 2, 3, 7, 8, 9, 15, 16, 17, 31, 32, 33,
			41, 42, 43, 63, 64, 65, 127, 128, 129, 255, 256, 257, 1023, 1024,
			1025, 4095, 4096, 4097, 16383, 65535, 65536, 65537, 100000};
	char				name[80];
	size_t				i;
	size_t				b;

	b = (size_t)BUFFER_SIZE;
	i = 0;
	while (i < sizeof(lengths) / sizeof(lengths[0]))
	{
		if (lengths[i] != b - 1 && lengths[i] != b && lengths[i] != b + 1)
		{
			snprintf(name, sizeof(name), "Long line: %zu chars, with newline",
				lengths[i]);
			run_pattern_case(name, lengths[i], 1, stats);
		}
		i++;
	}
}

static void	run_buffer_boundary_cases(t_stats *stats)
{
	size_t	b;
	char	name[100];

	b = (size_t)BUFFER_SIZE;
	if (b > 12000000)
	{
		printf("[SKIP] BUFFER_SIZE boundary stress: value exceeds 12000000\n");
		return ;
	}
	snprintf(name, sizeof(name), "Line length BUFFER_SIZE - 1 (%zu)", b - 1);
	run_pattern_case(name, b - 1, 1, stats);
	snprintf(name, sizeof(name), "Line length BUFFER_SIZE (%zu)", b);
	run_pattern_case(name, b, 1, stats);
	snprintf(name, sizeof(name), "Line length BUFFER_SIZE + 1 (%zu)", b + 1);
	run_pattern_case(name, b + 1, 1, stats);
}

static void	run_many_lines_case(t_stats *stats)
{
	const size_t	count = 1500;
	const size_t	capacity = 30000;
	char			*content;
	size_t			used;
	size_t			i;
	int				written;

	content = (char *)malloc(capacity);
	if (content == NULL)
	{
		stats->total++;
		stats->failed++;
		printf("[FAIL] 1500 short lines: test-data malloc failed\n");
		return ;
	}
	used = 0;
	i = 0;
	while (i < count)
	{
		written = snprintf(content + used, capacity - used, "line-%04zu\n", i);
		if (written < 0 || (size_t)written >= capacity - used)
		{
			free(content);
			stats->total++;
			stats->failed++;
			printf("[FAIL] 1500 short lines: test-data overflow\n");
			return ;
		}
		used += (size_t)written;
		i++;
	}
	run_case_data("1500 short lines in one file", content, used, stats);
	free(content);
}

static void	run_null_fd_case(const char *name, int fd, t_stats *stats)
{
	char	*line;

	stats->total++;
	line = get_next_line(fd);
	if (line == NULL)
	{
		stats->passed++;
		printf("[PASS] %s (NULL returned)\n", name);
	}
	else
	{
		stats->failed++;
		printf("[FAIL] %s: expected NULL, got: ", name);
		print_visible(line, strlen(line));
		printf("\n");
		free(line);
	}
}

static void	run_read_error_cases(t_stats *stats)
{
	char	path[] = "/tmp/gnl_writeonly_XXXXXX";
	int		temp_fd;
	int		write_fd;
	int		closed_fd;

	run_null_fd_case("Invalid descriptor -1", -1, stats);
	closed_fd = open("/dev/null", O_RDONLY);
	if (closed_fd >= 0)
	{
		close(closed_fd);
		run_null_fd_case("Closed file descriptor", closed_fd, stats);
	}
	temp_fd = mkstemp(path);
	if (temp_fd >= 0)
	{
		close(temp_fd);
		write_fd = open(path, O_WRONLY);
		unlink(path);
		if (write_fd >= 0)
		{
			run_null_fd_case("Write-only descriptor (read must fail)", write_fd,
				stats);
			close(write_fd);
		}
	}
	if (fcntl(42, F_GETFD) < 0 && errno == EBADF)
		run_null_fd_case("Invalid descriptor 42", 42, stats);
	else
		printf("[SKIP] Descriptor 42 is already open in this process\n");
}

static void	run_stdin_pipe_case(t_stats *stats)
{
	const char	content[] = "stdin-first\n\nstdin-third-without-newline";
	int			pipe_fd[2];
	int			saved_stdin;
	int			ok;

	stats->total++;
	if (pipe(pipe_fd) < 0)
	{
		stats->failed++;
		printf("[FAIL] STDIN pipe: pipe() failed\n");
		return ;
	}
	if (!write_all(pipe_fd[1], content, sizeof(content) - 1))
	{
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		stats->failed++;
		printf("[FAIL] STDIN pipe: write failed\n");
		return ;
	}
	close(pipe_fd[1]);
	saved_stdin = dup(STDIN_FILENO);
	if (saved_stdin < 0 || dup2(pipe_fd[0], STDIN_FILENO) < 0)
	{
		if (saved_stdin >= 0)
			close(saved_stdin);
		close(pipe_fd[0]);
		stats->failed++;
		printf("[FAIL] STDIN pipe: could not redirect stdin\n");
		return ;
	}
	close(pipe_fd[0]);
	ok = check_fd_content(STDIN_FILENO, "STDIN pipe", content, sizeof(content)
			- 1);
	dup2(saved_stdin, STDIN_FILENO);
	close(saved_stdin);
	if (ok)
	{
		stats->passed++;
		printf("[PASS] STDIN pipe (fd 0)\n");
	}
	else
		stats->failed++;
}

static void	run_return_before_eof_pipe_test(t_stats *stats)
{
	int				pipe_fd[2];
	pid_t			pid;
	pid_t			wait_result;
	int				status;
	int				timed_out;
	struct timespec	start;
	struct timespec	now;
	struct timespec	pause_time;
	char			*line;

	stats->total++;
	if (pipe(pipe_fd) < 0)
	{
		stats->failed++;
		printf("[FAIL] Return immediately after newline: pipe() failed\n");
		return ;
	}
	if (!write_all(pipe_fd[1], "FIRST\n", 6))
	{
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		stats->failed++;
		printf("[FAIL] Return immediately after newline: pipe write failed\n");
		return ;
	}
	pid = fork();
	if (pid < 0)
	{
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		stats->failed++;
		printf("[FAIL] Return immediately after newline: fork failed\n");
		return ;
	}
	if (pid == 0)
	{
		close(pipe_fd[1]);
		line = get_next_line(pipe_fd[0]);
		if (line == NULL || strcmp(line, "FIRST\n") != 0)
			_exit(1);
		free(line);
		close(pipe_fd[0]);
		_exit(0);
	}
	close(pipe_fd[0]);
	clock_gettime(CLOCK_MONOTONIC, &start);
	pause_time.tv_sec = 0;
	pause_time.tv_nsec = 10000000L;
	timed_out = 0;
	while (1)
	{
		wait_result = waitpid(pid, &status, WNOHANG);
		if (wait_result == pid)
			break ;
		if (wait_result < 0 && errno != EINTR)
			break ;
		clock_gettime(CLOCK_MONOTONIC, &now);
		if ((now.tv_sec - start.tv_sec) > 2 || ((now.tv_sec - start.tv_sec) == 2
				&& now.tv_nsec >= start.tv_nsec))
		{
			timed_out = 1;
			kill(pid, SIGKILL);
			waitpid(pid, &status, 0);
			break ;
		}
		nanosleep(&pause_time, NULL);
	}
	close(pipe_fd[1]);
	if (!timed_out && wait_result == pid && WIFEXITED(status)
		&& WEXITSTATUS(status) == 0)
	{
		stats->passed++;
		printf("[PASS] Returns after first newline without waiting for EOF\n");
	}
	else
	{
		stats->failed++;
		if (timed_out)
			printf("[FAIL] Returns after first newline: waited for EOF (timeout)\n");
		else
			printf("[FAIL] Returns after first newline: child returned an error\n");
	}
}

static void	print_summary(const t_stats *stats)
{
	printf("\n==================================================\n");
	printf("GNL TEST SUMMARY | BUFFER_SIZE = %d\n", BUFFER_SIZE);
	printf("Total:  %d\n", stats->total);
	printf("Passed: %d\n", stats->passed);
	printf("Failed: %d\n", stats->failed);
	printf("==================================================\n");
}

int	main(int argc, char **argv)
{
	t_stats	stats;
	size_t	i;
	size_t	b;

	static const t_case fixed_cases[] = {{"Empty file", ""},
											{"One newline only", "\n"},
											{"Two newline characters", "\n\n"},
											{"Three newline characters",
												"\n\n\n"},
											{"One character, no newline", "a"},
											{"One character with newline",
												"a\n"},
											{"Short word, no newline", "hello"},
											{"Short word with newline",
												"hello\n"},
											{"Two lines, no final newline",
												"first\nsecond"},
											{"Two lines, final newline",
												"first\nsecond\n"},
											{"Three lines, final newline",
												"one\ntwo\nthree\n"},
											{"Three lines, no final newline",
												"one\ntwo\nthree"},
											{"Empty line between words",
												"first\n\nlast"},
											{"Empty line at the beginning",
												"\nfirst"},
											{"Empty line at the end",
												"first\n\n"},
											{"Several empty lines at the end",
												"first\n\n\n\n"},
											{"Only spaces", "          "},
											{"Spaces as a complete line",
												"     \nnext\n"},
											{"Tabs as a complete line",
												"\t\t\nnext"},
											{"Mixed spaces and tabs",
												" \t  \t\n\t x \t\n"},
											{"Carriage return before newline",
												"first\r\nsecond\r\n"},
											{"Carriage return without newline",
												"first\r"},
											{"Two carriage returns and newline",
												"\r\r\n"},
											{"Punctuation and symbols",
												"!@#$%^&*()_+-=[]{};:,.?/\n"},
											{"Percent signs",
												"100%\n%printf%%\n"},
											{"Quotes and backslashes",
												"'single' \"double\" \\\n"},
											{"Leading and trailing spaces",
												"  hello world  \n"},
											{"Long word followed by short line",
												"abcdefghijklmnopqrstuvwxyz\nx\n"},
											{"Alternating blank and nonblank lines",
												"a\n\nb\n\nc\n"},
											{"Newline after every character",
												"a\nb\nc\nd\ne\nf\n"},
											{"No newline anywhere",
												"one two three four five"},
											{"UTF-8 text", "café\nمرحبا\n"},
											{"Line ending with multiple spaces",
												"word      \nnext\n"},
											{"Many tabs in one line",
												"\t\t\tvalue\t\tend\n"},
											{"Mixed newline placement",
												"\nA\n\nBC\n\nDEF"}};
	memset(&stats, 0, sizeof(stats));
	printf("GNL mandatory stress tests | BUFFER_SIZE = %d\n", BUFFER_SIZE);
	printf("No binary files and no multi-FD switching (bonus excluded).\n\n");
	if (argc > 1 && strcmp(argv[1], "--extreme-buffer") == 0)
	{
		run_case("Extreme mode: empty file", "", &stats);
		run_case("Extreme mode: newline only", "\n", &stats);
		run_case("Extreme mode: several lines", "first\nsecond\nthird", &stats);
		run_case("Extreme mode: no final newline", "last-line", &stats);
		b = (size_t)BUFFER_SIZE;
		if (b > 0 && b <= 12000000)
		{
			run_pattern_case("Extreme mode: BUFFER_SIZE - 1", b - 1, 1, &stats);
			run_pattern_case("Extreme mode: BUFFER_SIZE", b, 1, &stats);
			run_pattern_case("Extreme mode: BUFFER_SIZE + 1", b + 1, 1, &stats);
		}
		run_stdin_pipe_case(&stats);
		run_return_before_eof_pipe_test(&stats);
		run_read_error_cases(&stats);
		print_summary(&stats);
		return (stats.failed != 0);
	}
	run_return_before_eof_pipe_test(&stats);
	i = 0;
	while (i < sizeof(fixed_cases) / sizeof(fixed_cases[0]))
	{
		run_case(fixed_cases[i].name, fixed_cases[i].content, &stats);
		i++;
	}
	run_generated_cases(&stats);
	run_long_line_cases(&stats);
	run_buffer_boundary_cases(&stats);
	run_many_lines_case(&stats);
	run_stdin_pipe_case(&stats);
	run_read_error_cases(&stats);
	print_summary(&stats);
	return (stats.failed != 0);
}
