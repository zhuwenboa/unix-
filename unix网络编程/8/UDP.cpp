#include<iostream>
#include<fcntl.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<array>
#include<string.h>
#include<sys/time.h>
#include<netinet/in.h>

using namespace std;

int main(int argc, char *argv[])
{
    struct sockaddr_in ser_addr;
    struct sockaddr_in cli_addr;
    memset(&ser_addr, 0, sizeof(ser_addr));
    memset(&cli_addr, 0, sizeof(cli_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(atoi(argv[1]));
    ser_addr.sin_addr.s_addr = INADDR_ANY;
    int servfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(servfd < 0)
    {
        std::cout << "socket failed\n";
        return -1;
    }
    int ret;
    ret = bind(servfd, (sockaddr*)&ser_addr, sizeof(ser_addr));
    if(ret < 0)
    {
        std::cout << "bind failed\n";
        return -1;
    }
    char readbuf[1024], writebuf[1024];
    socklen_t cli_len = sizeof(cli_addr);
    while(1)
    {
        std::cout << "i am running\n";
        memset(readbuf, 0, sizeof(readbuf));
        memset(writebuf, 0, sizeof(writebuf));
        recvfrom(servfd, readbuf, 1024, 0, (sockaddr*)&cli_addr, &cli_len);
        std::cout << "readbuf: " << readbuf << std::endl;
        sprintf(writebuf, "recv your message thank you\n");
        sendto(servfd, writebuf, 1024, 0, (sockaddr*)&cli_addr, cli_len);
        close(servfd);
        break;
    }
}