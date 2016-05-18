#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>


#define REQUEST_MSIZE 1024

int pipe_rd;

typedef struct request_struct
{
	int pid;
	int size;
	char * call_dir;
	char * action;
	char ** targets;	
}request_struct;


typedef void (*sighandler_t)(int);

void signalhandler(int sign){
	if(sign == SIGINT){
		printf("\nServer closing\n");
		close(pipe_rd);
		unlink("/tmp/request_queue");
		exit(1);
	}
}


request_struct * requesthandler(char* client_request){
	
	int i;
	char* target = malloc(100*sizeof(char)); 
	request_struct *rs = malloc(sizeof(request_struct));

	rs->action = malloc(100*sizeof(char));
	rs->call_dir = malloc(100*sizeof(char));
	rs->targets = malloc(100*sizeof(char*));

	rs->pid = atoi(strtok(client_request," "));
	rs->size = atoi(strtok(NULL," "));
	rs->call_dir = strtok(NULL," "); // Não funciona se alguma pasta tiver espaços no nome
	rs->action = strtok(NULL," ");
	for(i=0;(target=strtok(NULL," "))!= NULL;i++){
		rs->targets[i]=strdup(target);
	}
	
	return rs;

}

int main(int argc, char const *argv[])
{
	signal(SIGINT,signalhandler);
	int read_bytes, i, fd[2];
	char request[REQUEST_MSIZE], aux[256];
	char * filename = malloc(256*sizeof(char));
	char * username = strdup(getpwuid(getuid())->pw_name);
	char dir[128];
	snprintf(dir, 128, "/home/%s/.Backup/pipe", username);

	
/* Remover pipes ou ficheiros com o nome a ser usado */
	unlink(dir);

/* Pipe onde chegam os pedidos */
	if (mkfifo(dir, 0777) < 0){
		puts("Couldn't create requested pipe");
		exit(-1);
	}

/* Abrir um descritor de ficheiros do pipe acima criado para leitura */
	if ((pipe_rd = open(dir, O_RDONLY)) == -1){
		perror("File Descriptor");
		exit(-1);
	}

/* Receber um pedido (bit stream) e fazer o que ele pede */
	for(;;){
		read_bytes = read(pipe_rd, request, REQUEST_MSIZE);
		// Alocação de memoria para a estrutura do request
		request_struct *rs = malloc(sizeof(request_struct));
		// Transformação da string para a estrutura
		rs=requesthandler(request);
		// Tratar do pedido para os filhos
		if(fork()==0){ // No filho
			// 2 Coisas a fazer: Fazer o pedido e enviar um signal resultado ao cli

			if(strcmp(rs->action,"backup")==0){

				for(i=0; rs->targets[i]!=NULL ;i++){
					pipe(fd);
					if(fork()==0){ // processo filho para executar sha1sum
						//printf("%s\n", strcat(rs->call_dir,rs->targets[i]) );
						dup2(fd[1],0);
						close(fd[0]);
						close(fd[1]);
						execlp("sha1sum", "sha1sum", strcat(rs->call_dir, rs->targets[i]), NULL); // -> verificar o strcat outra vez <-
						perror("Failed to execute sha1sum\n");
						//sinal a enviar ao cliente a avisar que falhou
						_exit(-1);
					} else { // processo pai 
						close(fd[1]); 

						//adicionar o wait para que o sha1sum seja feito antes do gzip and so on.
						
						// Receber resultado sha1sum, tirar o path à frente , apenas ficar com digest 
						read(fd[0], aux, 256);
						filename=strtok(aux," ");
						close(fd[0]);
						
						if(fork()==0){ // Processo filho para comprimir o ficheiro em questão.
							execlp("gzip", "gzip", rs->targets[i], NULL);
							perror("Failed to execute gzip");
							//sinal a enviar ao cliente a avisar que falhou
							_exit(-1);
						} else {/*
							if(fork()==0){// Processo filho para mover o ficheiro para a diretoria /home/user/.Backup/data
								execlp("mv", "mv", filename, "/home/user/.Backup/data/", NULL);
								perror("Failed to move file");
								//sinal a enviar ao cliente a avisar que falhou
								_exit(-1);								
							} else { // Processo pai escreve no ficheiro metadata a ligação para o ficheiro na diretoria /data
								char link_metadata[1024]/* = '\0'*//*;
								char digest_filename[256];
								int tam=strlen(filename);
								snprintf(link_metadata, tam, "%s -> %s", rs->targets[i], filename);

							}*/
						}
					}			
				}
			} else if(strcmp(rs->action,"restore")==0){


			}


		}// O processo pai simplesmente avança para o próximo pedido.
		
		
		close(pipe_rd);
		pipe_rd = open(dir, O_RDONLY);
	}


	
	return 0;
}

/*

CENAS POR FAZER: 

- Receber novos pedidos
- Mudar caminhos para dir e dir++data
- RESTORE


*/