#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <util.h>

#define size 1024

ssize_t readln(int fildes, void *buf, size_t nbyte)
{
	char *varchar = (char *) buf;
	int i=0;
	ssize_t x=0;
	
	while((x += read(fildes, varchar+i, size))>0 && i<nbyte){
		if (varchar[x-1] == '\n') break;
		i++;
	}
	return x;
}


int main(int argc, char const *argv[])
{
	mkfifo("queuePedidos", 0777); /* Pipe onde chegam os pedidos */
	int fd;
	fd = open("queuePedidos", O_RDONLY);
	if (fd == -1){
		perror("Descritor de ficheiros");
		return 1;
	}
 	char pedido[size];
	int x;
	while((x=read(fd, pedido, size))>0) { /* Ciclo à espera de pedidos dos clientes */
		pedido[x] = '\0';
		printf("%s", pedido);
	}
	if((close(fd))==-1){
		perror("Descritor de ficheiros");
		return 1;
	}
	return 0;
}