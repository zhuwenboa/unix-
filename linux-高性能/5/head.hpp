#ifndef _HEAD_H
#define _HEAD_H
#include<iostream>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>

void my_err(std::string str, int line)
{
    fprintf(stderr, "line:  %d\n", line);
    std::cerr << str << std::endl;
    exit(0);
}

#endif
