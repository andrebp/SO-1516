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
	int pipe_wr;
	char pedido[size];
	int i, n_bytes;

	if (argc < 3) { /* Argumentos insuficientes */
		puts("Error: Insufficient Arguments.");
		return 1;
	}
	
	if ((pipe_wr = open("request_queue", O_WRONLY)) == -1){
		perror("File Descriptor");
		return 1;
	}
	
	/* Concatenar argv numa só string pedido */
	for (i = 1; i < argc; i++) {
		strcat(pedido, argv[i]);
        if (argc > i+1)
        	strcat(pedido, " ");
    }
	n_bytes = strlen(pedido);
    
	/*Enviar o seu PID */

	pid_t mine_pid = getpid();
//	write(pipe_wr,&mine_pid,sizeof(pid_t));

    /* Enviar pedido pelo pipe */
	for (i = 0; i < 5; i++)
	{
	    write(pipe_wr, &n_bytes, sizeof(int));
	    write(pipe_wr, pedido, strlen(pedido));
	}
	sleep(10);
	signal(SIGUSR2,signalhandler);
	if((close(pipe_wr))==-1){
		perror("File Descriptor");
		return 1;
	}
	return 0;
}

/* 

- Cliente manda stream de bits com tudo.
- 4 bytes PID + 4 bytes n_bytes + nbytes = tamanho da stream de bites


Relatório:

- Fazer datagrama a explicar buffer;
-- Se a mensagem for mt grande enão couber no buffer á primeira deveria ler-se os primeiros 8 bits
para saber até onde se tem de ler.

*/