#include"head.hpp"

int main(int argc, char *argv[])
{
    if(argc < 3)
       my_err("command", __LINE__);
    int port = atoi(argv[2]);
    
    //设置连接的结构
    struct sockaddr_in serv_add;
    memset(&serv_add, 0, sizeof(serv_add));
    serv_add.sin_family = AF_INET;
    serv_add.sin_port = htons(port);
    serv_add.sin_addr.s_addr = inet_addr(argv[1]);

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0)
        my_err("socket", __LINE__);
    if(connect(sock_fd, (struct sockaddr*)&serv_add, sizeof(serv_add)) < 0)
        my_err("connect", __LINE__);
    const char *oob_data = "abc";
    const char *normal_data = "123";
    send(sock_fd, normal_data, strlen(normal_data), 0);
    send(sock_fd, oob_data, strlen(oob_data), MSG_OOB); //发送带外数据
    send(sock_fd,normal_data, strlen(normal_data), 0);

    close(sock_fd);
    
}