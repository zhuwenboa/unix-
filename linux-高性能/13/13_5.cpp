//消息队列 
//在进程间传递文件描述符
/*在两个不相关的进程中利用消息队列传递文件描述符*/

#include<iostream>
#include<sys/socket.h>
#include<fcntl.h>
#include<unistd.h>
#include<memory>
#include<string.h>
#include<assert.h>

static const int CONTROL_LEN = CMSG_LEN(sizeof(int));

//发送文件描述符，fd参数是用来传递信息的unix域socket，fd_to_send参数是待发送的文件描述符
void send_fd(int fd, int fd_to_send)
{
    /*
    struct iovec
    {
        ptr_t iov_base; //starting address
        size_t iov_len; //length in bytes
    };
    */
    struct iovec iov[1];
   
   
   
    /*
    struct msghdr 
     { 
        void  * msg_name ;   / *  消息的协议地址  * / 协议地址和套接口信息，在非连接的UDP中，
        发送者要指定对方地址端口，接受方用于的到数据来源，如果不需要的话可以设置为NULL（在TCP或者连接的UDP中，一般设置为NULL）
        socklen_t msg_namelen ;   / *  地址的长度  * / 
        struct iovec  * msg_iov ;   / *  多io缓冲区的地址  * / 
        int  msg_iovlen ;   / *  缓冲区的个数  * / 
        void  * msg_control ;   / *  辅助数据的地址  * / 
        socklen_t msg_controllen ;   / *  辅助数据的长度  * / 
        int  msg_flags ;   / *  接收消息的标识  * / 
    } ;
    */
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    
    
    /*
    truct cmsghdr 
    {
        socklen_t cmsg_len;
        int       cmsg_level;
        int       cmsg_type;
    };
    其成员描述如下：
    成员        描述
    cmsg_len    附属数据的字节计数，这包含结构头的尺寸。这个值是由CMSG_LEN()宏计算的。
    cmsg_level    这个值表明了原始的协议级别(例如，SOL_SOCKET)。
    cmsg_type    这个值表明了控制信息类型(例如，SCM_RIGHTS)。
    cmsg_data    这个成员并不实际存在。他用来指明实际的额外附属数据所在的位置。
    */
    cmsghdr cm;
    cm.cmsg_len = CONTROL_LEN;
    cm.cmsg_level = SOL_SOCKET;
    cm.cmsg_type = SCM_RIGHTS;
    *(int *)CMSG_DATA(&cm) = fd_to_send;
    msg.msg_control = &cm; //设置辅助数据
    msg.msg_controllen = CONTROL_LEN;

    sendmsg(fd, &msg, 0);
}

//接收目标文件描述符
int recv_fd(int fd)
{
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr cm;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;

    recvmsg(fd, &msg, 0);

    int fd_to_read =  *(int *)CMSG_DATA(&cm);
    return fd_to_read;
}

int main()
{
    int pipefd[2];
    int fd_to_pass = 0;
    //创建父、子进程间的管道，文件描述符pipefd[0]和pipefd[1]都是UNIX域socket
    int ret = socketpair(PF_UNIX, SOCK_DGRAM, 0, pipefd);
    assert(ret != -1);

    pid_t pid = fork();
    assert(pid >= 0);

    if(pid == 0)
    {
        close(pipefd[0]);
        fd_to_pass = open("test.txt", O_RDWR, 0666);
        //子进程通过管道将文件描述符发送到父进程。如果文件test.txt打开失败，则子进程将标准输入文件描述符发送到父进程
        send_fd(pipefd[1], (fd_to_pass > 0) ? fd_to_pass : 0);
        close(fd_to_pass);
        exit(0);
    }

    close(pipefd[1]);
    fd_to_pass = recv_fd(pipefd[0]); //父进程从管道接收文件描述符
    char buf[1024];
    memset(buf, '\0', sizeof(buf));
    read(fd_to_pass, buf, 1024);             //读目标文件描述符，以验证其有效性
    printf("I got fd %d and data %s\n", fd_to_pass, buf);
    close(fd_to_pass);
}