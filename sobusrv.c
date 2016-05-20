#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>


#define REQUEST_MSIZE 1024
#define MAX_ACTIVE_REQUESTS 5

/* Variáveis usadas como globais são mexidas apenas pelo processo principal do servidor e pelo signalhandler.
	De outra forma era impossivel usar sinais para as situações que usamos */

int pipe_rd;
char pipe_path[128];
int active_requests=0;

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
	if(sign == SIGUSR1){
		printf("Sinal recebido 1\n");
		printf("%d\n", active_requests );
		active_requests=active_requests-1;
		printf("%d\n", active_requests );
	} else 	if(sign == SIGINT){

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
	signal(SIGUSR1,signalhandler);
	int read_bytes, i, fd[2], main_pid, son_pid, status;
	char request[REQUEST_MSIZE];
	char * username = strdup(getpwuid(getuid())->pw_name);
	char root_path[128];
	char data_path[128];
	char metadata_path[128];
	snprintf(root_path, 128, "/home/%s/.Backup/", username);
	snprintf(pipe_path, 128, "%spipe", root_path);
	snprintf(data_path, 128, "%sdata/", root_path);
	snprintf(metadata_path, 128, "%smetadata/", root_path);

	main_pid=getpid();
	
/* Remover pipes ou ficheiros com o nome a ser usado */
	unlink(pipe_path);

/* Pipe onde chegam os pedidos */
	if (mkfifo(pipe_path, 0777) < 0){
		printf("Couldn't create requested pipe\n");
		exit(-1);
	}

/* Abrir um descritor de ficheiros do pipe acima criado para leitura */
	if ((pipe_rd = open(pipe_path, O_RDONLY)) == -1){
		perror("File Descriptor");
		exit(-1);
	}

/* Receber um pedido (bit stream) e fazer o que ele pede */
	for(;;){
		// Impedir o avanço da execução se o número de pedidos ativos for ==MAX_ACTIVE_REQUESTS ou incrementar o contador de pedidos ativos caso contrário.
		while(active_requests >= MAX_ACTIVE_REQUESTS){
			wait(NULL);
		}
		// Ler pedido do pipe
		request[0]='\0';
		read_bytes = read(pipe_rd, request, REQUEST_MSIZE);
		// Alocação de memoria para a estrutura do request
		request_struct *rs = malloc(sizeof(request_struct));
		// Transformação da string para a estrutura
		rs=requesthandler(request);
		printf("Active request antes de incrementar %d\n", active_requests);
		active_requests=active_requests+1;
		printf("Active request depois de incrementar %d\n", active_requests );
		// A execução do pedido fica a cargo de um processo requestHandler que trata do pedido recebido e sinaliza o cliente do sucesso ou insucesso.
		if(fork()==0){ /* Processo requestHandler */

			if(strcmp(rs->action,"backup")==0){

				for(i=0; rs->targets[i]!=NULL ;i++){
					pipe(fd);
					char target_path[128];
					snprintf(target_path, 128, "%s%s", rs->call_dir, rs->targets[i]);
					if((son_pid=fork())==0){ // processo filho para executar sha1sum
						dup2(fd[1],1);
						close(fd[0]);
						close(fd[1]);
						execlp("sha1sum", "sha1sum", target_path, NULL); 
						perror("Failed to execute sha1sum\n");
						_exit(-1);
					} else { // Processo requestHandler
						close(fd[1]); 
						if(waitpid(son_pid, &status, 0)==son_pid && WEXITSTATUS(status)!=0){
							kill(rs->pid, SIGUSR2);
							continue;
						}
						// Receber resultado sha1sum, tirar o path à frente , apenas ficar com digest 
						char aux[128];
						read(fd[0], aux, 128);
						char * digest = malloc(128*sizeof(char));
						digest=strtok(aux," ");
						close(fd[0]);
						
						if((son_pid=fork())==0){ // Processo filho para comprimir o ficheiro em questão.
							execlp("gzip", "gzip","-k" ,target_path, NULL);
							perror("Failed to execute gzip");
							_exit(-1);

						} else { // Processo requestHandler

							if(waitpid(son_pid, &status, 0)==son_pid && WEXITSTATUS(status)!=0){
								kill(rs->pid, SIGUSR2);
								continue;
							}

							char move_path[256];
							snprintf(move_path, 256, "%s%s.gz", data_path, digest);
							strcat(target_path,".gz");
							if((son_pid=fork())==0){// Processo filho para mover o ficheiro para a diretoria /home/user/.Backup/data/ com o nome do digest
								execlp("mv", "mv", target_path, move_path, NULL);
								perror("Failed to move file");
								//sinal a enviar ao cliente a avisar que falhou <-----
								_exit(-1);								
							} else { // // Processo requestHandler
								
								
								if(waitpid(son_pid, &status, 0)==son_pid && WEXITSTATUS(status)!=0){
									kill(rs->pid, SIGUSR2);
									/*executar um rm para limpar o ficheiro comprimido que não foi movido */
									continue;
								}


								if((son_pid=fork())==0){//Processo filho para criar o link na diretoria /home/user/.Backup/metadata/ com o nome do digest
									char link_path[256];
									snprintf(link_path, 256, "%s%s", metadata_path, rs->targets[i]);
									execlp("ln", "ln", "-f", "-s", "-T", move_path, link_path, NULL);
									printf("Couldn't create symlink\n");
									_exit(-1);
								} else { // // Processo requestHandler
									
									if(waitpid(son_pid, &status, 0)==son_pid && WEXITSTATUS(status)!=0){
										kill(rs->pid, SIGUSR2);
										/* Executar um rm para limpar o ficheiro na pasta data */
										continue;
									}

									/* Sinal ao cliente que este ficheiro correu bem bem */	
									kill(rs->pid, SIGUSR1);		
								}
							}
						}
					}
				}
				// Processo que atende o pedido, requesthandler, sai, enviando um sinal ao processo principal que decrementa os pedidos ativos.
				kill(main_pid, SIGUSR1);
				_exit(0); 
			} else if(strcmp(rs->action,"restore")==0){
				/* Percorrer todos os pedidos */
				for(i=0;rs->targets[i] != NULL; i++){
					pipe(fd);
					if((son_pid=fork())==0){ //processo filho que vai buscar o path do link do ficheiro a fazer restore
						dup2(fd[1],1);
						close(fd[0]);
						close(fd[1]);
						char link_path[256];
						snprintf(link_path, 256, "%s%s", metadata_path, rs->targets[i]);
						execlp("readlink", "readlink", "-f", "-n", link_path, NULL); // flag '-n' serve para tirar o \n no final da string
						printf("Couldn't obtain link\n");
						_exit(-1);
					} else { // Processo requestHandler
						close(fd[1]);

						if(waitpid(son_pid, &status, 0)==son_pid && WEXITSTATUS(status)!=0){
							kill(rs->pid, SIGUSR2);
							continue;
						}

						char * symbolic_link = malloc(256*sizeof(char));
						read(fd[0],symbolic_link,256); //path da diretoria data/ onde está o ficheiro a ser restored
						close(fd[0]);

						if((son_pid=fork())==0){ // Processo filho para descomprimir o ficheiro

							execlp("gunzip", "gunzip", "-f", "-k", symbolic_link, NULL);
							perror("Failed to execute gunzip");
							_exit(-1);

						} else { // Neste ponto, o ficheiro está restaurado, mas encontra-se na diretoria data, move-se agora para a diretoria de trabalho do user
							
							if(waitpid(son_pid, &status, 0)==son_pid && WEXITSTATUS(status)!=0){
							kill(rs->pid, SIGUSR2);
							continue;
						}

							if((son_pid=fork())==0){ // processo filho para mover o ficheiro para a diretoria de trabalho
								symbolic_link[strlen(symbolic_link)-3]='\0'; // Retirar o .gz, o mv deve ser aplicado ao ficheiro e não ao ficheiro comprimido
								strcat(rs->call_dir,rs->targets[i]);
								execlp("mv", "mv", symbolic_link, rs->call_dir, NULL);
								perror("Failed to move file");
								_exit(-1);
							} else { // Processo requestHandler
								
								if(waitpid(son_pid, &status, 0)==son_pid && WEXITSTATUS(status)!=0){
									kill(rs->pid, SIGUSR2);
									continue;
								}
								
								kill(rs->pid, SIGUSR1); /* Sinalizar ao cliente que a operação para este ficheiro correu bem */
							}
						}
					}
				}
				// Processo que atende o pedido, requesthandler, sai, enviando um sinal ao processo principal que decrementa os pedidos ativos.
				kill(main_pid, SIGUSR1);
				_exit(0); 
			}
		}
		/* Processo principal do servidor simplesmente avança para o próximo pedido 
				abrindo e fechando o pipe de modo a bloquear novamente, e criando um novo requestHandler pro novo pedido */
		free(rs);
		close(pipe_rd);
		pipe_rd = open(pipe_path,O_RDONLY);
	}
	return 0;
}

/*

CENAS POR FAZER: 

TESTAR COM VÁRIOS FICHEIROS!!!


- SINAL AO CLIENTE SE CORRER MAL 	<- CHECKAR COMO ESTÁ.
- ROLLBACK NAS CENAS MAL 			<- Dá muito trabalho.

Relatório:
Conices (Capa, Indice, Objetivos -> interpretação do enunciado, esturtura do relatório -> como é que ele foi desenvolvido)
Explicar estrutura geral dos programas.
Dizer pontos fulcrais do programa.
Dizer como os resolvemos.
Conclusão e trabalho futuro

PONTOS IMPORTANTES A REFERIR: 
- IMPEDIR MAIS DE 5 LIGAÇÕES		<- 	Variável global guarda o número de pedidos ativos.
										Sempre que um requestHandler é criado o valor é incrementado,
										sempre que um requestHandler acaba o trabalho o valor é decrementado através dum sinal,
										que é enviado ao processo principal.
										Caso numero de requests ativos >5:
										- Poe-se o main process à espera que um request saia.
..... + ?

- TRABALHO FUTURO -> CRIAR UMA TABELA ONDE NUMERO DE SIGNALS == SITIO ESPECIFICO DA FALHA DA EXECUÇÂO


*/