
all: cliente servidor

cliente: sobucli.c
	gcc -o sobucli sobucli.c

servidor: sobusrv.c
	gcc -o sobusrv sobusrv.c

clean:
	rm sobucli sobusrv
