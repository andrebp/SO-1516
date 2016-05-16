#include "util.h"


ssize_t readln(int fildes, void *buf, size_t nbyte)
{
	char *varchar = (char *) buf;
	int i=0;
	ssize_t x=0;
	
	while((x += read(fildes, varchar+i, 1024))>0 && i<nbyte){
		if (varchar[x-1] == '\n') break;
		i++;
	}
	return x;
}