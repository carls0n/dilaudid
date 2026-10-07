#include <fcntl.h>
#include <syscall.h>
#include <unistd.h>

int main(void) {
    if (syscall(SYS_truncate, "/etc/ld.so.preload", 0LL) < 0)
        return syscall(SYS_write, 2, "Error\n", 6), 1;

    return 0;
}

