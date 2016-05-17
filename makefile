CC = gcc

default: cliente servidor

cliente: sobucli.o 
	$(CC) -o sobucli sobucli.o

sobucli.o: sobucli.c
	$(CC) -c sobucli.c

servidor: sobusrv.o 
	$(CC) -o sobusrv sobusrv.o

sobusrv.o: sobusrv.c
	$(CC) -c sobusrv.c

utilitarios: utils.c utils.h
	$(CC) util.c -o util

clean:
	rm sobucli sobusrv request_queue *.o
