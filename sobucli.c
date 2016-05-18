#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <util.h>

#define REQUEST_MSIZE 1024
#define REQUEST_HEADER 10

/* Função que produz uma mensagem com (PID do processo cliente + Tamanho Comando + Comando) */
int produce_request(char * comando, char *request)
{	 
	int cmd_bytes, process_pid, tamanho;
	
	// Construir o datagrama: 1) PID + 2) Tamanho Comando + 3) Comando 
	// 1) PID
	process_pid = (int)getpid();

 	// 2) Tamanho Comando
	cmd_bytes = strlen(comando);
	printf("%s, %d\n", comando, cmd_bytes);
	
	// 3) A string comando é criada na main

	// Por 1), 2) e 3) na string request
	snprintf(request, REQUEST_HEADER, "%d %d ", process_pid, cmd_bytes);
	strcat(request, comando);
	tamanho=strlen(request);

	return tamanho;
}


int main(int argc, char const *argv[])
{
	int pipe_wr;
	char request[REQUEST_MSIZE];
	char comando[REQUEST_MSIZE-REQUEST_HEADER];
	int i, request_size;

 // Argumentos insuficientes 
	if (argc < 3) {
		puts("Error: Insufficient Arguments.");
		return 1;
	}

// Abrir um descritor de ficheiros do pipe criado pelo servidor para escrita
	if ((pipe_wr = open("/tmp/request_queue", O_WRONLY)) == -1){
		perror("File Descriptor");
		exit(-1);
	}

// Construir string Comando a partir do argv
	comando[0]='\0';
	for (i = 1; i < argc; i++) {
		strcat(comando, argv[i]);
        if (argc > i+1)
        	strcat(comando, " ");
    }

// Preparar request/mensagem 
	request_size = produce_request(comando, request);
 
 // Enviar request pelo pipe
	write(pipe_wr, request, request_size+1);
	
	close(pipe_wr);
	return 0;
}


/* 

Relatório:

- Fazer datagrama a explicar buffer;
-- Se a mensagem for mt grande enão couber no buffer á primeira deveria ler-se os primeiros 8 bits
para saber até onde se tem de ler.

*/