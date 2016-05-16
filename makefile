CC = gcc

all: cliente servidor 

cliente: sobucli.c 
	$(CC) sobucli.c -o sobucli

servidor: sobusrv.c 
	$(CC) sobusrv.c -o sobusrv

utilitarios: utils.c utils.h
	$(CC) util.c -o util

clean:
	rm sobucli sobusrv queuePedidos
