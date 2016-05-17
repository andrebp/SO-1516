CC = gcc

default: cliente servidor

cliente: sobucli.o 
	$(CC) -o cliente sobucli.o

sobucli.o: sobucli.c
	$(CC) -c sobucli.c

servidor: sobusrv.o 
	$(CC) -o servidor sobusrv.o

sobusrv.o: sobusrv.c
	$(CC) -c sobusrv.c

utilitarios: utils.c utils.h
	$(CC) util.c -o util

clean:
	rm sobucli sobusrv queuePedidos
