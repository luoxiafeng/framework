#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NETLINK_TEST         30
#define PAYLOAD_MAX_SIZE    1024

struct test_nlmsg {
    struct nlmsghdr nlh;
    uint8_t    msg_data[PAYLOAD_MAX_SIZE];
};

int netlink_socket = -1;
struct sockaddr_nl  *user_addr = NULL;      // self address
struct sockaddr_nl  *kernel_addr = NULL;    // target address
struct test_nlmsg    *msg = NULL;            // message buffer

int main ()
{
    // Create netlink socket
    netlink_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if (netlink_socket == -1)
    {
        perror("Socket create failed!\n");
        return -1;
    }

    // fill msg head info
    msg = (struct test_nlmsg *)malloc(sizeof(struct test_nlmsg));
    if(msg == NULL)
    {
        perror("msg malloc failed!\n");
        close (netlink_socket);
        return -1;
    }
    memset(msg, 0, sizeof(struct test_nlmsg));
    msg->nlh.nlmsg_len = sizeof(struct test_nlmsg);
    msg->nlh.nlmsg_pid = getpid();  // sender's pid
    msg->nlh.nlmsg_flags = 0;   // no special flag

    // fill sender's addr
    user_addr = (struct sockaddr_nl *)malloc(sizeof(struct sockaddr_nl));
    if(user_addr == NULL)
    {
        perror("user_addr malloc failed!\n");
        close (netlink_socket);
        free(msg);
        return -1;
    }
    memset(user_addr, 0, sizeof(struct sockaddr_nl));
    user_addr->nl_family = AF_NETLINK;   // Netlink socket
    user_addr->nl_pid = getpid();        // user space process pid
    user_addr->nl_groups = 0;            // no multicast

    // fill receive's addr
    kernel_addr = (struct sockaddr_nl *)malloc(sizeof(struct sockaddr_nl *));
    if(kernel_addr == NULL)
    {
        perror("kernel_addr malloc failed!\n");
        close (netlink_socket);
        free(msg);
        free(user_addr);
        return -1;
    }
    memset(kernel_addr, 0, sizeof(struct sockaddr_nl));
    kernel_addr->nl_family = AF_NETLINK;
    kernel_addr->nl_pid = 0;     // kernel process pid
    kernel_addr->nl_groups = 0;

    // bind socket
    int ret = bind(netlink_socket, (struct sockaddr *) user_addr, sizeof(struct sockaddr_nl));
    if (ret == -1)
    {
        perror("bind failed!\n");
        close (netlink_socket);
        free(msg);
        free(user_addr);
        free(kernel_addr);
        return -1;
    }

    // fill msg
    char *buf = "hello netlink!";
    memset(&(msg->msg_data), 0, PAYLOAD_MAX_SIZE);
    strcpy(msg->msg_data, buf);

    // send msg
    printf("Send message to kernel\n");
    ssize_t send_len = sendto(netlink_socket, msg, msg->nlh.nlmsg_len, 0, (struct sockaddr *)kernel_addr, sizeof(struct sockaddr_nl));
    if(send_len == -1)
    {
        perror("send failed!\n");
        close (netlink_socket);
        free(msg);
        free(user_addr);
        free(kernel_addr);
        return -1;
    }

    ssize_t recv_len = -1;
    struct test_nlmsg recv_msg;
    // recv msg
    socklen_t kernel_addrlen = sizeof(struct sockaddr_nl);
    recv_len = recvfrom(netlink_socket, &recv_msg, sizeof(struct test_nlmsg), 0, (struct sockaddr *)kernel_addr, &kernel_addrlen);
    if(recv_len == -1)
    {
        perror("recv failed!\n");
        free(msg);
        free(user_addr);
        free(kernel_addr);
        close (netlink_socket);
        return -1;
    }
    printf("Recv from kernel: %s\n", recv_msg.msg_data);

    // release
    close(netlink_socket);
    free(msg);
    free(user_addr);
    free(kernel_addr);

    return 0;
}