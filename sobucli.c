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

int num_op, current_op=1;

typedef void (*sighandler_t)(int);
char** filenames;//[50][128];

void signalhandler(int sign){
	if(sign == SIGUSR2){
		printf("%s: Ocorreu um erro.\n", filenames[current_op]);// <--- 2) SE DE FACTO FUNCIONAR, É SO TIRAR O COMENTARIO PARA IMPRIMIR DIREITO.
		current_op++;
	}
	else if(sign == SIGUSR1){
		printf("%s: Sucesso.\n", filenames[current_op]);//  <--- 3) SE DE FACTO FUNCIONAR, É SO TIRAR O COMENTARIO PARA IMPRIMIR DIREITO.
		current_op++;
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
 	 	
	// 4) A string comando é criada na main

	// 2) Tamanho Info
	info_bytes = strlen(comand) + strlen(c_work_dir) + 3; // +2 Separador (espaço) + / + '\0'

	// Por tudo na string request
	snprintf(request, REQUEST_MSIZE, "%d %d %s/ %s", process_pid, info_bytes, c_work_dir, comand);
	printf("%s\n", request);

	tamanho = strlen(request) + 1; // +1 Para o carater de terminação
	return tamanho;
}

int main(int argc, char const *argv[])
{
	signal(SIGUSR2,signalhandler);
	signal(SIGUSR1,signalhandler);
	int pipe_wr;
	char request[REQUEST_MSIZE];
	char comand[REQUEST_MSIZE-15];
	int i, j, request_size;
	char * username = strdup(getpwuid(getuid())->pw_name);
	char dir[128];
	snprintf(dir, 128, "/home/%s/.Backup/pipe", username);
	filenames = malloc(50*sizeof(char*));

// Número de operações/ficheiros
	num_op = argc - 2;
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

// Error check: Comando inválido
	if ((strcmp(argv[1], "backup") != 0) && (strcmp(argv[1], "restore") != 0)){
		printf("Invalid comand, try again.\n");
		exit(-1);
	}

// Construir string Comando a partir do argv
	// Adicionar separador depois do comando
	comand[0]='\0';
	strcat(comand, " ");
	for (i = 1, j = 0; i < argc; i++, j++) {
		//strcpy(argv[i], filenames[j]); //<--- 1) SE DE FACTO FUNCIONAR, É SO TIRAR O COMENTARIO PARA GUARDAR OS NOMES. 
		filenames[j] = strdup(argv[i]);
		strcat(comand, argv[i]);
        if (argc > i+1)
        	strcat(comand, " ");
    }

// Preparar request/mensagem 
	request_size = produce_request(comand, request);

// Enviar request pelo pipe
	write(pipe_wr, request, request_size);
	
// Esperar pelos sinais do servidor

	close(pipe_wr);

	while(current_op <= num_op);
	return 0;
}


/* 

-- CENAS POR FAZER NO CLIENTE:

TESTAR COM VÁRIOS FICHEIROS!!!

- RECEBER SINAIS NO GERAL.
- TRADUZIR EXPRESSÕES REGULARES.


Relatório:

- Fazer datagrama a explicar buffer;
-- Se a mensagem for mt grande enão couber no buffer á primeira deveria ler-se os primeiros 8 bits
para saber até onde se tem de ler.

*/