//用非阻塞connect进行连接的客户端
/*
说明：由于程序用select等待连接完成，可以设置一个select等待时间限制，从而缩短connect超时时间。
多数实现中，connect的超时时间在75秒到几分钟之间。有时程序希望在等待一定时间内结束，
使用非阻塞connect可以防止阻塞75秒，在多线程网络编程中，尤其必要。
例如有一个通过建立线程与其他主机进行socket通信的应用程序，如果建立的线程使用阻塞connect与远程通信，
当有几百个线程并发的时候，由于网络延迟而全部阻塞，阻塞的线程不会释放系统的资源，同一时刻阻塞线程超过一定数量时候，
系统就不再允许建立新的线程（每个进程由于进程空间的原因能产生的线程有限），如果使用非阻塞的connect，连接失败使用select等待很短时间，
如果还没有连接后，线程立刻结束释放资源，防止大量线程阻塞而使程序崩溃。
*/

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<iostream>
#include<error.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<signal.h>
#include<sys/select.h>
int main(int argc, char* argv[])
{
    fd_set rset, wset;
    struct timeval tval;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int flag = fcntl(sockfd, F_GETFL);
    //将套接字设置为非阻塞
    fcntl(sockfd, F_SETFL, flag | O_NONBLOCK);

    struct sockaddr_in client_addr; 
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(atoi(argv[2]));
    client_addr.sin_addr.s_addr = inet_addr(argv[1]);

    int ret = connect(sockfd, (sockaddr*)&client_addr, sizeof(client_addr));
    if(ret == 0)
        std::cout << "socket connect succeed immdiately\n";
    else
    {
        std::cout << "get the connect result by select(). \n";
        if(errno != EINPROGRESS)
            return -1; //连接失败
        else if(errno == EINPROGRESS)
        {
            FD_ZERO(&rset);
            FD_SET(sockfd, &rset);
            wset = rset;
            tval.tv_sec = 2;
            tval.tv_usec = 0;
            ret = select(sockfd + 1, &rset, &wset, NULL, &tval);
            if(ret < 0)
            {
                perror("select erro\n");
                exit(-1);
            }
            if(ret == 0)
            {
                //超过timeval的时间还没有连接成功
                close(sockfd);
                errno = ETIMEDOUT;
                exit(errno);
            }
            else if(FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
            {
                
                int error = 0;
                socklen_t len = sizeof(error);

                //如果error为0则表明connect成功连接，关闭其在select中写描述符集对应的位。
                ret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
                if(ret < 0)
                {
                    close(sockfd);
                    return -1;
                }
                if(error != 0)
                {
                    perror("connect failed\n");
                    close(sockfd);
                    exit(-1);
                }
                else
                {
                    std::cout << "connect success\n";
                }
            }
        }
    }
    //连接成功后设置为阻塞模式
    fcntl(sockfd, F_SETFL, flag);
    /*
    ...
    */
    close(sockfd);
}