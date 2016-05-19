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
char pipe_path[128];

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
		unlink(pipe_path);
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
	char * digest = malloc(128*sizeof(char));
	char * username = strdup(getpwuid(getuid())->pw_name);
	char root_path[128];
	char data_path[128];
	char metadata_path[128];
	snprintf(root_path, 128, "/home/%s/.Backup/", username);
	snprintf(pipe_path, 128, "%spipe", root_path);
	snprintf(data_path, 128, "%sdata/", root_path);
	snprintf(metadata_path, 128, "%smetadata/", root_path);
	
/* Remover pipes ou ficheiros com o nome a ser usado */
	unlink(pipe_path);

/* Pipe onde chegam os pedidos */
	if (mkfifo(pipe_path, 0777) < 0){
		puts("Couldn't create requested pipe");
		exit(-1);
	}

/* Abrir um descritor de ficheiros do pipe acima criado para leitura */
	if ((pipe_rd = open(pipe_path, O_RDONLY)) == -1){
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
					char target_path[128];
					snprintf(target_path, 128, "%s%s", rs->call_dir, rs->targets[i]);
					if(fork()==0){ // processo filho para executar sha1sum
						dup2(fd[1],0);
						close(fd[0]);
						close(fd[1]);
						execlp("sha1sum", "sha1sum", target_path, NULL); // -> verificar o strcat outra vez  <-----
						perror("Failed to execute sha1sum\n");
						_exit(-1);
					} else { // processo pai 
						close(fd[1]); 

						//adicionar o wait para que o sha1sum seja feito antes do gzip ou parar caso corra mal (SIGNAL) <----- CONTINUE ;P
						
						// Receber resultado sha1sum, tirar o path à frente , apenas ficar com digest 
						read(fd[0], aux, 256);
						digest=strtok(aux," ");
						close(fd[0]);
						
						if(fork()==0){ // Processo filho para comprimir o ficheiro em questão.
							/* FALTA NAO APAGAR O FICHEIRO ORIGEM */
							execlp("gzip", "gzip", target_path, NULL);
							perror("Failed to execute gzip");
							_exit(-1);

						} else {
							//adicionar o wait para que o sha1sum seja feito antes do move ou parar cenas caso corra mal.(SIGNAL) <----- CONTINUE ;P

							if(fork()==0){// Processo filho para mover o ficheiro para a diretoria /home/user/.Backup/data/ com o nome do digest
								char move_path[256];
								snprintf(move_path, 256, "%s%s", root_path, digest);
								execlp("mv", "mv", target_path, move_path, NULL);
								perror("Failed to move file");
								//sinal a enviar ao cliente a avisar que falhou <-----
								_exit(-1);								
							} else { // Processo pai escreve no ficheiro metadata a ligação para o ficheiro na diretoria /metadata

								//adicionar o wait para que o move seja feito antes dum novo cenas ou parar caso corra mal.(SIGNAL) <----- CONTINUE ;P

								if(fork()==0){//Processo filho para criar o link na diretoria /home/user/.Backup/metadata/ com o nome do digest
									char link_path[256];
									snprintf(link_path, 256, "%s%s", metadata_path, digest);
									execlp("ln", "ln", "-s", move_path, link_path, ,NULL);
									puts("Couldn't create symlink");
									_exit(-1);
								} else { // Processo pai espera que a operação do filho acabe enviando um resultado ao cliente (SIGNAL)
 
									//adicionar o wait
								}
							}
						}
					}

					/* Sinal ao cliente que correu bem */			
				
				}
			} else if(strcmp(rs->action,"restore")==0){


			}


		}// O processo pai simplesmente avança para o próximo pedido abrindo e fechando o pipe de modo a bloquear novamente.
		close(pipe_rd);
		pipe_rd = open(pipe_path,O_RDONLY);
	}
	return 0;
}

/*

CENAS POR FAZER: 

- FALTA GZIP NAO FODER O FICHEIRO DE ORIGEM
- RESTORE
- IMPEDIR O PROCESSO DE CONTINUAR SE DER MERDA A MEIO
- SINAL AO CLIENTE SE CORRER MAL


*/