#include <linux/init.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/list.h>

struct cache_node_info {
    int key;
    struct rb_node node;
};

static void insertRBTree(struct rb_root *root,struct cache_node_info *my_node)
{
    struct rb_node **new = &root->rb_node;
    struct rb_node *parent = NULL;
    //iterate root node,find the node that we insert at
    //the node we find must be NULL,or we can't insert it,beacuse the node is null,new->rb_node is null
    //so we let new->rb_node = node,node insert success
    while (*new) {
        struct cache_node_info *entry = container_of(*new,struct cache_node_info,node);
        parent = *new;
        if (my_node->key > entry->key)
            new = &((*new)->rb_right);
        else if (my_node->key < entry->key)
            new = &((*new)->rb_left);
    }
    rb_link_node(&my_node->node,parent,new);
    rb_insert_color(&my_node->node,root);
}

static void searchRBTreeByNode(struct rb_root *root,struct cache_node_info *my_node)
{
        struct rb_node* node = root->rb_node;
        struct cache_node_info *entry = NULL;
        while (node) {
            entry = container_of(node,struct cache_node_info,node);
            if (my_node->key > entry->key)
                node = node->rb_right;
            else if (my_node->key < entry->key)
                node = node->rb_left;
            else {
                printk("find the node,key = %d\n",entry->key);
                return;
            }
        }
}

static void searchRBTreeByKey(struct rb_root* root,int key)
{
        //get the node of the root
        struct rb_node* node = root->rb_node;
        struct cache_node_info *entry = NULL;
        while(node) {
            entry = container_of(node,struct cache_node_info,node);
            if (key > entry->key)
                node = node->rb_right;
            else if (key < entry->key)
                node = node->rb_left;
            else {
                printk("find the node,key = %d\n",entry->key);
                return;
            }     
        }
        printk("didn't find the node as key=%d\n",key);
}

static void deleteRBTreeNode(struct rb_root* root,int key)
{
        //search node
        struct rb_node* node = root->rb_node;
        struct cache_node_info *entry = NULL;
        while (node) {
            entry = container_of(node,struct cache_node_info,node);
            if (key > entry->key)
                node = node->rb_right;
            else if (key < entry->key)
                node = node->rb_left;
            else {
                rb_erase(node,root);
                //rb_erase_color(node,root);
                kfree(node);
                printk("delete node success,key = %d\n",key);
                return;
            }
        }
}

static void displayRBTree(struct rb_root* root)
{
    struct rb_node* node = NULL;
    struct cache_node_info *entry = NULL;
    int i = 0;
    for (node = rb_first(root); node; node = rb_next(node)) {
        struct cache_node_info *entry = container_of(node,struct cache_node_info,node);
        printk("key = %d \t",entry->key);
        if (i % 10 == 0)
            printk("\n");
        i++;
    }
    printk("total : %d\n",i);
}
static int getRBTreeDepth(struct rb_root* root)
{
    struct rb_node* node = NULL;
    int depth = 0;
    for (node = rb_first(root); node; node = rb_next(node)) {
        depth++;
    }
    return depth;
}
/*
*(1)module_param(name,type,perm)
*(2)insmod model.ko value=200
*(3)cat /sys/module/model/parameters/value
*/
static int __init hello_init(void)
{
        struct rb_root root = RB_ROOT;
        struct cache_node_info* my_node_p = NULL;
        struct cache_node_info search_node;
        int i = 0;
        printk(KERN_EMERG "hello, init\n");
        for (i = 0; i < 100; i++) {
            my_node_p = kmalloc(sizeof(struct cache_node_info),GFP_KERNEL);
            my_node_p->key = i + 1;
            insertRBTree(&root,my_node_p);
        }
        displayRBTree(&root);
        searchRBTreeByKey(&root,50);
        searchRBTreeByKey(&root,100);
        searchRBTreeByKey(&root,101);
        search_node.key = 50;
        searchRBTreeByNode(&root,&search_node);
        //delete the node from the tree
        printk("delete 1.3.5.7.9... node\n");
        for (i = 0; i < 100; i+=2) {
            deleteRBTreeNode(&root,i+1);
        }
        displayRBTree(&root);
        int depth = getRBTreeDepth(&root);
        printk("the deep of the tree is %d\n",depth);
        return 0;
}
 
static void __exit hello_exit(void)
{
        printk(KERN_EMERG "hello, exit\n");
}
 
module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiafeng.luo");
MODULE_DESCRIPTION("This is just a hello module!\n");


/*Makefile for cross-compile 
obj-m := test.o

KDIR := /home/rlk/xiafeng.luo/buildroot-2022.11-rc2/linux/linux-6.0.9/
CROSS_COMPILE := /home/rlk/xiafeng.luo/CrossCompileTools/gcc-linaro-12.2.1-2022.11-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
all:
    make -C $(KDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
    rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions modules.* Module.* *.mod
*/