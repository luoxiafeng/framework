#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <linux/sched/debug.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/types.h>
#include <linux/slab.h>

#define NETLINK_TEST 30
#define PAYLOAD_MAX_SIZE    1024

pid_t pid = -1;    
struct sock *netlink_socket = NULL;
EXPORT_SYMBOL_GPL(netlink_socket);
 

struct test_nlmsg {
    struct nlmsghdr nlh;
    u8    msg_data[PAYLOAD_MAX_SIZE];
};


// 收到信息的回调函数
static void netlink_message_handle(struct sk_buff *skb)
{
    // get test_nlmsg
    struct test_nlmsg *msg = (struct test_nlmsg *) skb->data;
    // set user process process pid
    pid = msg->nlh.nlmsg_pid;
    // 收到信息的 nlmsghdr 位于 msg->nlh
    // 收到信息的 payload 位于 msg->msg_data

    printk("Netlink info get!\n");

    // 构建发送信息结构体
    struct sk_buff *skb_out = nlmsg_new(PAYLOAD_MAX_SIZE, GFP_KERNEL);  // 使用该函数分配的内存空间会在 nlmsg_unicast 后自动释放
    struct nlmsghdr *nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, PAYLOAD_MAX_SIZE, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    // fill echo data
    strcpy((char *)NLMSG_DATA(nlh), msg->msg_data);

    nlmsg_unicast(netlink_socket, skb_out, pid);
}


static int test_socket_create(void)
{
    // set message receive callback function
    struct netlink_kernel_cfg cfg = {
        .input = netlink_message_handle,
    };

    // 创建 netlink socket
    netlink_socket = (struct sock *)netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);

    if(netlink_socket == NULL)
    {
        printk("Socket Create Failed!\n");
        return -1;
    }
    printk("Socket Create Succeed!\n");
    return 0;
}


static void test_socket_close(void)
{
    // release socket
    if (netlink_socket){
        netlink_kernel_release(netlink_socket);
        netlink_socket = NULL;
    }   
    printk("Socket Release Succeed!\n");

}


module_init(test_socket_create);
module_exit(test_socket_close);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiafeng.luo");