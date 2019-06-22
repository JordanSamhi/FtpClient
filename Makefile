all: ftpClient

ftpClient: ftpClient.o
	gcc -o ftpClient -lnsl  -Wall ftpClient.o
ftpClient.o: ftpClient.c
	gcc -c -Wall ftpClient.c

