#include<stdio.h>

void my_err(char *p, int line)
{
	fprintf(stderr, "line: %d", line);
	perror(p);
	exit(0);
}
