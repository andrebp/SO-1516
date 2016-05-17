#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
//#include <util.h>

#define REQUEST_MSIZE 1024

typedef void (*sighandler_t)(int);

void signalhandler(int sign){
	if(sign== SIGINT){
		printf("\nServer closing\n");
		exit(1);
	}
}

int main(int argc, char const *argv[])
{
	signal(SIGINT,signalhandler);
	int pipe_rd, read_bytes;
	char request[REQUEST_MSIZE];

/* Remover pipes ou ficheiros com o nome a ser usado */
	unlink("/tmp/request_queue");

/* Pipe onde chegam os pedidos */
	if (mkfifo("/tmp/request_queue", 0777) < 0){
		puts("Couldn't create request pipe");
		exit(-1);
	}

/* Abrir um descritor de ficheiros do pipe acima criado para leitura */
	if ((pipe_rd = open("/tmp/request_queue", O_RDONLY)) == -1){
		perror("File Descriptor");
		exit(-1);
	}

/* Receber um pedido (bit stream) e fazer o que ele pede */
	for(;;){
		read_bytes = read(pipe_rd, request, REQUEST_MSIZE);
		if (read_bytes <= 0 ) break;
		write(1, request, read_bytes);
	}

	close(pipe_rd);
	unlink("/tmp/request_queue");
	
	return 0;
}

/*

- Recebe-se um stream de bytes;
- Fork e processa-se;

// Ler o tamanho da string 
read(pipe_rd, &n_bytes, sizeof(int)); 

// Ler a string 
if((read_bytes = read(pipe_rd, request, n_bytes)) > 0){ 
	request[n_bytes] = '\0';
	printf("Pedido: %s, Tamanho: %d;\n", request, n_bytes);
} else { 	
	puts("No Read");
	end_of_execution = 1;
	printf("%d\n", read_bytes);
}



*/