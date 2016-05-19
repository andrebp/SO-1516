#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#define REQUEST_MSIZE 1024

int controlo = 1; /* alterar esta variavel para o numero de pedidos do cliente. Assim, o programa sobucli só termina quando todos acabarem e não apenas quando 1 acaba*/

typedef void (*sighandler_t)(int);

void signalhandler(int sign){
	if(sign == SIGUSR2){
		printf("Ocorreu um erro.\n");
		controlo = 0;
	}
	else if(sign == SIGUSR1){
		printf("Operação concluida com sucesso\n");
		controlo = 0;
	}
}


/* Função que produz uma mensagem com (PID do processo cliente +  Tamanho da Info + Diretoria de Trabalho + Comando) e devolve o
	tamanho em bytes 
*/
int produce_request(char * comand, char *request)
{	 
	int info_bytes, process_pid, tamanho;
	char c_work_dir[256];
	
	// Construir o datagrama: 1) PID + 2) Tamanho da Info + 3) Diretoria de Trabalho + 4) Comando 
	// 1) PID
	process_pid = (int)getpid();
	
	// 3) Diretoria de Trabalho
	getcwd(c_work_dir, sizeof(c_work_dir));
	c_work_dir[strlen(c_work_dir)]='/'; c_work_dir[strlen(c_work_dir)+1]='\0';

	// 4) A string comando é criada na main

	// 2) Tamanho Info
	info_bytes = strlen(comand) + strlen(c_work_dir) + 2; // +2 Separador (espaço) + '\0'

	// Por tudo na string request
	snprintf(request, REQUEST_MSIZE, "%d %d %s %s", process_pid, info_bytes, c_work_dir, comand);
	printf("%s\n", request);

	tamanho = strlen(request) + 1; // +1 Para o carater de terminação /home/USER/.Backup/
	return tamanho;
}

int main(int argc, char const *argv[])
{
	signal(SIGUSR2,signalhandler);
	signal(SIGUSR1,signalhandler);
	int pipe_wr;
	char request[REQUEST_MSIZE];
	char comand[REQUEST_MSIZE-15];
	int i, request_size;
	char * username = strdup(getpwuid(getuid())->pw_name);
	char dir[128];
	snprintf(dir, 128, "/home/%s/.Backup/pipe", username);

// Argumentos insuficientes 
	if (argc < 3) {
		puts("Error: Insufficient Arguments.");
		return 1;
	}

// Abrir um descritor de ficheiros do pipe criado pelo servidor para escrita
	if ((pipe_wr = open(dir, O_WRONLY)) == -1){
		perror("File Descriptor");
		exit(-1);
	}

// Construir string Comando a partir do argv
	comand[0]='\0';
	for (i = 1; i < argc; i++) {
		strcat(comand, argv[i]);
        if (argc > i+1)
        	strcat(comand, " ");
    }

// Preparar request/mensagem 
	request_size = produce_request(comand, request);

// Enviar request pelo pipe
	write(pipe_wr, request, request_size);
	
// Esperar pelos sinais do cenas

	close(pipe_wr);

	while(controlo==1);
	return 0;
}


/* 

-- CENAS POR FAZER NO CLIENTE:

RECEBER SINAIS,
TRADUZIR EXPRESSÕES REGULARES.

Relatório:

- Fazer datagrama a explicar buffer;
-- Se a mensagem for mt grande enão couber no buffer á primeira deveria ler-se os primeiros 8 bits
para saber até onde se tem de ler.

*/