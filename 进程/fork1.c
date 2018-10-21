#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>

int main()
{
	int fd;
	if( (fd = open("temp.txt", O_CREAT | O_RDWR, 0644)) < 0)
		perror("open file erro");
	pid_t pid;
	pid = fork();
	switch(pid)
	{
		case 0:
			if(write(fd, "hello", 6) < 0)
				perror("write file erro");
			break;
		case -1:
			perror("fork erro");
			break;
		default:
			wait(&pid);
			if(write(fd, "word!\n", 7) < 0)
				perror("parent write file erro");
			break;
	}
	exit(0);
}
