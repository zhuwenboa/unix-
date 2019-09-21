#include"head.hpp"

class Net  
{
public: 
    Net() = default;
    Net(int s)  : serv_fd(s) {}
    void Socket();
    void Bind(int port, char *ip);
    void Listen();
    int Accept();
private:  
    int serv_fd;
//    int cli_fd;

};

void Net::Socket()
{
    serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_fd < 0)
        my_err("socket", __LINE__);
}

void Net::Bind(int port, char *ip)
{
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv.sin_addr);
    
    int ret;
    ret = bind(serv_fd, (struct sockaddr*)&serv, sizeof(serv));
    if(ret < 0)
        my_err("bind", __LINE__);
}

void Net::Listen()
{
    int ret = listen(serv_fd, 5);
    if(ret < 0)
        my_err("listen", __LINE__);
}

int Net::Accept()
{
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    int fd = accept(serv_fd, (struct sockaddr*)&cli, &len);
    if(fd < 0)
        my_err("accpet", __LINE__);
    return fd;
}

class Epoll 
{
public: 

    void add_ctl(int sockfd, bool flag);
    void Wait();
    
private: 

};