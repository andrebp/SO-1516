CC = gcc

default: cliente servidor
	sudo mv ./sobusrv ./sobucli /bin/
	
cliente: sobucli.o 
	$(CC) -o sobucli sobucli.o

sobucli.o: sobucli.c
	$(CC) -c sobucli.c

servidor: sobusrv.o 
	$(CC) -o sobusrv sobusrv.o

sobusrv.o: sobusrv.c
	$(CC) -c sobusrv.c

installF: header installing cliente servidor
	sudo mkdir /home/$(USER)/.Backup/
	sudo mkdir /home/$(USER)/.Backup/data/
	sudo mkdir /home/$(USER)/.Backup/metadata/
	sudo mv ./sobusrv ./sobucli /bin/
	rm *.o

install: cliente servidor
	sudo mkdir /home/$(USER)/.Backup/
	sudo mkdir /home/$(USER)/.Backup/data/
	sudo mkdir /home/$(USER)/.Backup/metadata/
	sudo mv ./sobusrv ./sobucli /bin/
	rm *.o

uninstallF: header uninstalling
	sudo rm /bin/sobucli /bin/sobusrv
	sudo rm -r /home/$(USER)/.Backup/

uninstall:
	sudo rm /bin/sobucli /bin/sobusrv
	sudo rm -r /home/$(USER)/.Backup/

header:
	@echo "************************************"
	@echo "*     Efficient Backup System      *"
	@echo "************************************"
	
installing:
	@echo "*          Installing...           *"
	@echo "************************************"

uninstalling:
	@echo "*         Uninstalling...          *"
	@echo "************************************"

clean:
	rm sobucli sobusrv *.o
