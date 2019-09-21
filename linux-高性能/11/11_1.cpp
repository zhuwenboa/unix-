/*设置超时时间*/

#include"head.hpp"
#include<errno.h>

/*超时连接函数*/
int timeout_connect(const char *ip, int port, int time)
{
    int ret = 0;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv.sin_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);


    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;
    socklen_t len = sizeof(timeout);

    /*通过选项SO_RCVTIMEO和SO_SNDTIMEO所设置的超时时间的类型是timeval, 这和select系统调用的超时参数类型相同*/
    ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);

    ret = connect(sockfd, (struct sockaddr*)&serv, sizeof(serv));
    if(ret == -1)
    {
        /*超时对应的错误号是EINPROGRESS。下面这个条件如果成立，我们就可以处理定时任务了*/
        if(errno == EINPROGRESS)
        {
            printf("connecting timeout, process timeout logic\n");
            return -1;
        }
        printf("error occur when connecting to server\n");
        return -1;
    }
    return sockfd;
}
