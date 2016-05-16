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
	mkfifo("queuePedidos", 0666); /* Pipe onde chegam os pedidos */
	FILE * file;
	if((file = fopen("./queuePedidos", "r")) == NULL){
		perror("Caminho do ficheiro Inválido");
		return 1;
	}
	int fd;
	if((fd = fileno(file)) == -1) {
		perror("Stream de Ficheiro Inválido");
		return 1;
	}
	while(true) { /* Ciclo à espera de pedidos dos clientes */
		// ESTE CICLO FAZ SENTIDO ASSIM? -> NEVES
	 	char pedido[size];
		readln(fd, pedido, size);
		printf("%s\n", pedido);
	}
	return 0;
}