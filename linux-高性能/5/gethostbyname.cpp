#include<iostream>
#include"head.hpp"
#include<memory>

int main(int argc, char **argv)
{
    char *ip = argv[1];
    size_t  len =  strlen(ip) + 1;
    size_t len2 = sizeof(ip);
    std::cout << "len:"  << len << "\n" << "len2:" << len2 << std::endl;
    //初始化hostent指针
    /*
    struct hostent* gethostbyname(const char* name);
    struct hostent* gethostbyaddr(const void* addr, size_t len, int type);
    */

    struct hostent* hostname = gethostbyaddr(ip,  len, AF_INET);
    std::cout << "主机名：" << hostname->h_name   << std::endl;

}