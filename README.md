*This project has been created as part of the 42 curriculum by hes-sofy.*

# get_next_line

## Description

**get_next_line** is a C project that consists of implementing a function capable of reading from a file descriptor one line at a time.

The function returns one line on each call. It reads data progressively until it finds a newline character (`\n`) or reaches the end of the file (EOF). Any data remaining after the extracted line is preserved for the next call.

The project aims to develop an understanding of file descriptors, the `read()` system call, static variables, and dynamic memory management using `malloc()` and `free()`.

## Instructions

### Compilation

The project does not include a Makefile. Compile the source files directly with `cc`:

```bash
cc -Wall -Wextra -Werror get_next_line.c get_next_line_utils.c main.c -o gnl
```

A `main.c` file is required for testing and is not included in the project. It should call `get_next_line()` using a valid file descriptor.

### Buffer Size

The default value of `BUFFER_SIZE` is `10`, as defined in `get_next_line.h`.

You can override this value at compilation time using the `-D` flag. For example:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=1 get_next_line.c get_next_line_utils.c main.c -o gnl
```

You can replace `1` with another positive value.

### Execution

After successful compilation, run the program:

```bash
./gnl
```

The behavior depends on the test code in `main.c`. Pass the file descriptor of an opened file or standard input to `get_next_line()` and process the returned lines until the function returns `NULL`.

## Algorithm

This implementation reads data progressively and stores it temporarily until a complete line can be returned. The process follows these steps:

1. **Input validation:** At the beginning of each call, `get_next_line()` checks that the file descriptor is non-negative and that `BUFFER_SIZE` is positive. If either condition is not met, it frees the stored data and returns `NULL`.

2. **Preserving unread data:** A static pointer named `str` stores data that has not yet been returned. Because it is static, its value is preserved between calls. Before reading more data, the function checks whether the stored content already contains a newline. If it does, no additional read is needed.

3. **Reading:** The `ft_read_stash()` helper allocates a temporary buffer of `BUFFER_SIZE + 1` bytes and calls `read()` to read up to `BUFFER_SIZE` bytes at a time. After each successful read, it adds a null terminator to the buffer. Reading continues until a newline is found in the newly read data, EOF is reached, or a read error occurs.

4. **Appending data:** Each newly read chunk is appended to the stored content using `append_buffer()`. This helper uses `ft_strjoin()` to allocate a new string containing both the previous stash and the new buffer. The previous stash is then freed.

5. **Extracting a line:** Once reading stops, `extract_line()` copies the first line from the stash into a newly allocated string. The returned line includes the newline character when one is present. If EOF is reached without a newline, the remaining content is returned as the final line.

6. **Cleaning the stash:** After extracting the line, `clean_stash()` creates a new string containing only the data after the first newline. The old stash is freed, and the static pointer is updated so the remaining data can be used by the next call. If a newline is found, the new stash contains everything after it; this may be an empty string. If no newline is found in the stash, `clean_stash()` returns `NULL`.

### Why this algorithm?

This approach allows the function to handle lines longer than `BUFFER_SIZE`, because it can perform multiple read operations and accumulate their contents before extracting a line. It also handles cases where a single read contains more than one line: the first line is returned, while the remaining data is kept in the stash for subsequent calls.

Using a static stash preserves unread data between calls, while dynamic memory allocation allows the stored content to grow according to the amount of data read.

## Resources

### Technical References

- [`read()` — Linux manual](https://man7.org/linux/man-pages/man2/read.2.html): documentation on reading data from a file descriptor.
- [`malloc()` — Linux manual](https://man7.org/linux/man-pages/man3/malloc.3.html): documentation on dynamic memory allocation.
- [`free()` — Linux manual](https://man7.org/linux/man-pages/man3/free.3.html): documentation on releasing dynamically allocated memory.
- [C storage duration — cppreference](https://en.cppreference.com/w/c/language/storage_duration.html): reference on storage duration and static variables in C.

### AI Usage

Various AI tools were used as learning and development aids during the project. They helped with:

- **Understanding concepts:** clarifying `read()`, static variables, and dynamic memory management.
- **Code review:** discussing the implementation logic and examining potential errors, including memory-management issues.
- **Testing:** suggesting test cases and discussing their results to check the function's behavior in different situations.

These tools were used to support learning and analysis. The implementation and technical decisions were reviewed and understood during the development process.