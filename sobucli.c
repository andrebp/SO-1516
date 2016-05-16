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
	if (argc < 3) { /* Argumentos insuficientes */
		puts("Erro: Argumentos insuficientes.");
		return 1;
	}
	/* Abrir pipe de pedidos para escrita */
	int fd;
	fd = open("queuePedidos", O_WRONLY);
	if (fd == -1){
		perror("Descritor de ficheiros");
		return 1;
	}
	/* Concatenar argv numa só string pedido */
	char pedido[size];
	int i;
	for (i=1; i<argc; i++) {
		strcat(pedido, argv[i]);
        if (argc > i+1)
        	strcat(pedido, " ");
    }
    /* Enviar pedido pelo pipe */
    //printf("Pedido: %s ; Tamanho: %d ;\n", pedido, (int)strlen(pedido));
    write(fd, pedido, strlen(pedido));

	return 0;
}