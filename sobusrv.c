#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <util.h>

#define size 1024


int main(int argc, char const *argv[])
{
	int pipe_rd;
	char request[size];
	int n_bytes, read_bytes;
	short end_of_execution = 0;

	mkfifo("request_queue", 0777); /* Pipe onde chegam os pedidos */
	
	if ((pipe_rd = open("request_queue", O_RDONLY)) == -1){
		perror("File Descriptor");
		return 1;
	}

	while(!end_of_execution){
		read(pipe_rd, &n_bytes, sizeof(int)); /* Ler o tamanho da string */ 
		if((read_bytes = read(pipe_rd, request, n_bytes)) > 0){ 	/* Ler a string */
			request[n_bytes] = '\0';
			printf("Pedido: %s, Tamanho: %d;\n", request, n_bytes);
		} else { 	
			puts("No Read");
			end_of_execution = 1;
			printf("%d\n", read_bytes);
		}
	}

	if((close(pipe_rd))==-1){
		perror("File Descriptor");
		return 1;
	}
	return 0;
}

/*

- Recebe-se um stream de bytes;
- Fork e processa-se;


*/