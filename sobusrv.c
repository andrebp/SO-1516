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

int pipe_rd;

typedef struct request_struct
{
	int pid;
	int size;
	char* action;
	char** targets;	
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

	rs->action=malloc(100*sizeof(char));
	rs->targets=malloc(100*sizeof(char*));

	rs->pid = atoi(strtok(client_request," "));
	rs->size = atoi(strtok(NULL," "));
	rs->action = strtok(NULL," ");
	for(i=0;(target=strtok(NULL," "))!= NULL;i++){
		rs->targets[i]=strdup(target);
		printf("%s\n",rs->targets[i]);
	}
	
	return rs;

}

int main(int argc, char const *argv[])
{
	signal(SIGINT,signalhandler);
	int read_bytes, i, fd[2];
	char request[REQUEST_MSIZE], aux[256];
	char* filname = malloc(256*sizeof(char));
	char* usrname = strdup(getenv("USER"));
	char* backup_path= malloc(1024*sizeof(char));
	backup_path= "/home/";
	

	
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
						dup2(fd[0],1)
						close(fd[0]);
						close(fd[1]);
						execlp("sha1sum", "sha1sum", strcat(rs->path, rs->targets[i]), NULL); // -> verificar o strcat outra vez <-
						perror("Failed to execute sha1sum\n");
						//sinal a enviar ao cliente a avisar que falhou
						_exit(-1);
					} else { // processo pai 
						close(fd[1]); 
						// Receber resultado sha1sum, tirar o path à frente , apenas ficar com digest 
						read(fd[0], aux, 256);
						filename=strtok(aux," ");
						
						if(fork()==0){ // Processo filho para comprimir o ficheiro em questão.
							execlp("gzip", "gzip", filename, NULL);
							perror("Failed to execute gzip");
							//sinal a enviar ao cliente a avisar que falhou
							_exit(-1);
						} else {
							if(fork()==0){// Processo filho para mover o ficheiro para a diretoria /home/user/.Backup/data
								execlp("mv", "mv", filename, "/home/user/.Backup/data", NULL);
								perror("Failed to move file");
								//sinal a enviar ao cliente a avisar que falhou
								_exit(-1);								
							} else { // Processo pai escreve no ficheiro metadata a ligação para o ficheiro na diretoria /data
								char link_metadata[1024] = '\0';
								char digest_filename[]
								int tam=strlen(filename)
								snprintf(link_metadata, tam, "%s -> %s", rs->targets[i], filename);

							}
						}

						close(fd[0]);
					}			

				}

			} else if(strcmp(rs->action,"restore")==0){


			}


		}// O processo pai simplesmente avança para o próximo pedido.
		
		
		close(pipe_rd);
		pipe_rd = open("/tmp/request_queue",O_RDONLY);
	}


	
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