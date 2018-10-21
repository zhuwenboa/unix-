#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>

int globvar = 6;
char buf[] = "a write to stdout";

int main(void)
{
	int var;
	pid_t pid;
	var = 88;
	if(write(STDOUT_FILENO, buf, sizeof(buf) - 1) != sizeof(buf) - 1)
		perror("write erro");
	printf("before fork\n");

	if( (pid = fork()) < 0)
		perror("fork erro");
	else if(pid == 0)	//子进程运行
	{
		globvar++;
		var++;
	}
	else	//父进程等待子进程结束
		wait(&pid);
	printf("pid = %ld, glob = %d, var = %d\n", (long)getpid(), globvar, var);
	exit(0);
}
