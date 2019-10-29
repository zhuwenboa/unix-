#include<iostream>
#include<fcntl.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<array>
#include<string.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<poll.h>
#include<limits.h>
#include<signal.h>

using namespace std;

#define port 8888
#define max_conn 64

struct pollfd con[max_conn];
static int max_fd = 0;
static int listenfd;
//设置为非阻塞
void setnonblock(const int &fd)
{
    int old_op = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, old_op | O_NONBLOCK);
}

//信号处理函数
void handler(int sig)
{
    std::cout << "sig_hanler run \n";
    close(listenfd);
    exit(1);
}

int main(int argc, char *argv[])
{

    struct sockaddr_in ser_addr, cli_addr;
    //客户连接
    int connfd;
    memset(&ser_addr, 0, sizeof(ser_addr));
    memset(&cli_addr, 0, sizeof(cli_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    ser_addr.sin_addr.s_addr = INADDR_ANY;
    //accept中用到
    socklen_t cli_len = sizeof(cli_addr);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        cout << "socket failed\n";
        return -1;
    }
    setnonblock(listenfd);
    //加入到poll集合中
    con[0].fd = listenfd;
    con[0].events = POLLRDNORM; //处理该套接字可读事件

    int on;
    //setsockopt(listenfd, SOCK_)

    int ret = bind(listenfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
    if(ret < 0)
    {
        cout << "bind failed\n";
        return -1;
    }
    ret = listen(listenfd, 5);
    if(ret < 0)
    {
        cout << "listen failed\n";
        return -1;
    }
    //屏蔽ctrl+c
    signal(SIGINT, handler);

    char readbuf[1024], sendbuf[1024];
    int cond, i;
    for(i = 1; i < max_conn; ++i)
        con[i].fd = -1;
    while(true)
    {
        cond = poll(con, max_fd + 1, -1);
        std::cout << "cond = " << cond << "\n";
        if(cond < 0)
        {
            std::cout << "unonblock\n";
            sleep(1);
        }
        //有新的连接到来
        if(con[0].revents & POLLRDNORM)    
        {
            connfd = accept(con[0].fd, (sockaddr*)&cli_addr, &cli_len);
           // if(count < max_conn && con[count].fd == -1)
            for(i = 1; i < max_conn; ++i)
            {
                if(con[i].fd == -1)
                {
                    con[i].fd = connfd;
                    setnonblock(con[i].fd);
                    con[i].events = POLLRDNORM;
                    break;
                }
            }
            if(i > max_fd)
                max_fd = i;
            if(i == max_conn)
            {
                char str[] = "connection is full, sorry\n";
                send(connfd, str, strlen(str) + 1, 0);
            }
            if(--cond <= 0)
                continue;
        }
        //客户有数据到达
        for(i = 1; i < max_conn; ++i) 
        {
            if(con[i].fd < 0)
                continue;
            //有数据可读
            if(con[i].revents & POLLRDNORM)
            {
                memset(readbuf, 0, sizeof(readbuf));
                ret = recv(con[i].fd, readbuf, 1024, 0);
                if(ret < 0)
                {
                    if(errno == ECONNRESET)
                    {
                        //倘若发送端发送缓冲区有数据未发送完或者接受缓冲区有数据未读完，则会导致ECONNRESET，我们在接收端将其关闭
                        //connection reset by client;
                        close(con[i].fd);
                        con[i].fd = -1;
                    }
                    else
                    {
                        perror("recv failed\n");
                        
                        //return -1;
                    }
                }
                //断开连接
                if(ret == 0)
                {
                    close(con[i].fd);
                    con[i].fd = -1;
                }
                else
                {
                    std::cout << "recv data: " << readbuf << endl;
                }
            }
            if(--cond <= 0)
                break;
        }   
    }
    for(i = 0; i < max_conn; ++i)
        close(con[i].fd);
    return 0;
}