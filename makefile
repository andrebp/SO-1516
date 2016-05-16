CC = gcc


all: cliente servidor 

cliente: sobucli.c 
	$(CC) -o sobucli sobucli.c

servidor: sobusrv.c 
	$(CC) -o sobusrv sobusrv.c

utilitarios: utils.c utils.h
	$(CC) -o util utils

clean:
	rm sobucli sobusrv
