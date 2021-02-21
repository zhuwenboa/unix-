#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<iostream>
#include<string.h>
#include<unistd.h>
#include<sys/sem.h>
#include<sys/wait.h>
#include<memory>
#include<sys/types.h>

//mmap映射的文件不能为空，否则进行写操作时会产生总线错误。
//对mmap返回的指针进行写操作，只能在文件原有大小基础上进行写入。

class Mmap
{
public:  
    Mmap(const char* file);
    ~Mmap();
    bool wirte(char*);
    bool read(int);
    int Fd();

private:    
    int fd;
    char* share_mem;
};

Mmap::Mmap(const char* file)
{
    fd = open(file, O_CREAT | O_RDWR, 0666);
    if(fd < 0)
    {
        std::cout << "opne file failed\n";
    }
    char temp_mes[1024] = {0};
    //write(fd, temp_mes, 1024);
    ftruncate(fd, 1024);
    share_mem = (char*)mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    //fsync(fd);
}

Mmap::~Mmap()
{
    close(fd);
    int ret = munmap((void*)share_mem, 1024);
    if(ret != 0)
    {
        std::cout << "munmap failed\n";
    }
}

bool Mmap::wirte(char* mes)
{
    strncpy(share_mem, mes, 1024);
    return true;
}

bool Mmap::read(int size)
{
    if(size > 5 * 1024)
        return false;
    char read_content[1024] = {0};
    //strncpy(read_content, share_mem, size);
    ::read(fd, read_content, 1024);
    std::cout << read_content << "\n";
    return true;
}

int main()
{
    Mmap test("1.txt");
    char* mes = "zhuwenbo";
    test.wirte(mes);
    test.read(1000);
}