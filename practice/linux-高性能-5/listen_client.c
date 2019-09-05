#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

void my_err(char *str, int line)
{
   fprintf(stderr, "line:%d\n", line);
   perror(str);
   exit(0);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    socklen_t len;
    int cli_fd;
    if( (cli_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        my_err("socket", __LINE__);
    
    int port = atoi(argv[2]);

    /*设置连接结构*/
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    len = sizeof(struct sockaddr_in);
    if(connect(cli_fd, (struct sockaddr *)&serv_addr, len) < 0)
        my_err("connect", __LINE__);
    printf("connect success\n");
    sleep(10);


}