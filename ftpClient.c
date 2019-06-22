#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <time.h> 
#include <ifaddrs.h>
#include <arpa/inet.h>

#define P 21
#define LGMES 200
#define MAX_SEND_SIZE 1000

int getCode(char reponseServ[]);
int getRandomPort();
int creerSockListen(u_short port, int *isLocalhostConnection);
int getPasvPort(char reponseServ[]);
int getNbArgsCmd(char cmd[]);
void getPasvAddr(char *reponseServ, char *adresseDistante);
int cmdNeedTwoChannels(char cmd[]);
void getUserCmd(char *cmd, char *argsCmd);
void getLocalAddressForActiveRequest(char *address, int *isLocalhostConnection);
void connectToFtpServ(int *sock, struct hostent *recup, struct sockaddr_in adresse, u_short *port, char *reponseServ, int *isLocalhostConnection);
void authentificationFtpServ(int *sock, char *reponseServ);
void passiveConnection(int *sock, int *sockPassif, char *reponseServ, struct sockaddr_in adressePassif);
void printBanner();
void displayHelp();
void preparingActiveMode(int *sockActif, int *sock, char *reponseServ, int *isLocalhostConnection);
void lsCmd(int *sock, int *sockActif, int *sockPassif, int *passiveMode, char *reponseServ);
void cdCmd(int * sock, char *argsCmd, char *reponseServ);
void deleteCmd(int *sock, char *argsCmd, char *reponseServ);
void mkdirCmd(int *sock, char *argsCmd, char *reponseServ);
void rmdirCmd(int *sock, char *argsCmd, char *reponseServ);
void renameCmd(int *sock, char *argsCmd, char *reponseServ);
void sendTypeBin(int *isTypeBinSent, int *sock, char *reponseServ);
void getCmd(int *sock, int *sockActif, int *sockPassif, int *isTypeBinSent, char *reponseServ, int *passiveMode, char *argsCmd);
void putCmd(int *sock, int *sockActif, int *sockPassif, int *isTypeBinSent, char *reponseServ, int *passiveMode, char *argsCmd);
void systemCmd(int *sock, char *reponseServ);
void passiveCmd(int *passiveMode);
void quitCmd(int *sock, int *sockActif, int *sockPassif, char *reponseServ);
void usage();
void argsManagement(int *argc, char *arg, u_short *port);
void displayPrompt(char *cmd);

int main(int argc, char *argv[]) {
	int sock, sockActif, sockPassif, passiveMode = 0, isTypeBinSent = 0, isLocalhostConnection = 0;
	char cmd[BUFSIZ], reponseServ[BUFSIZ], argsCmd[BUFSIZ];
	u_short port;
	struct sockaddr_in adresse, adressePassif;	
	struct hostent *recup = NULL;
	srand((unsigned) time(NULL));
	
	argsManagement(&argc, argv[1], &port);
	printBanner();
	connectToFtpServ(&sock, recup, adresse, &port, reponseServ, &isLocalhostConnection);
	authentificationFtpServ(&sock, reponseServ);
	
	while(1){
		displayPrompt(cmd);
		if(getNbArgsCmd(cmd) > 1)
        	getUserCmd(cmd, argsCmd);
		if(cmdNeedTwoChannels(cmd)){
			if(passiveMode)
				passiveConnection(&sock, &sockPassif, reponseServ, adressePassif);
			else
				preparingActiveMode(&sockActif, &sock, reponseServ, &isLocalhostConnection);
		}
		if(strlen(cmd) != 0){
			if (strcmp(cmd, "ls") == 0)
			  	lsCmd(&sock, &sockActif, &sockPassif, &passiveMode, reponseServ);
			else if (strcmp(cmd, "cd") == 0)
				cdCmd(&sock, argsCmd, reponseServ);
			else if(strcmp(cmd,"delete") == 0)
				deleteCmd(&sock, argsCmd, reponseServ);
			else if(strcmp(cmd,"mkdir") == 0)
				mkdirCmd(&sock, argsCmd, reponseServ);
			else if(strcmp(cmd,"rmdir") == 0)
				rmdirCmd(&sock, argsCmd, reponseServ);
			else if(strcmp(cmd,"rename") == 0)
				renameCmd(&sock, argsCmd, reponseServ);
			else if(strcmp(cmd,"get") == 0)
				getCmd(&sock, &sockActif, &sockPassif, &isTypeBinSent, reponseServ, &passiveMode, argsCmd);
			else if (strcmp(cmd, "put") == 0)
				putCmd(&sock, &sockActif, &sockPassif, &isTypeBinSent, reponseServ, &passiveMode, argsCmd);
			else if (strcmp(cmd, "system") == 0)
				systemCmd(&sock, reponseServ);
			else if (strcmp(cmd, "passive") == 0)
				passiveCmd(&passiveMode);
			else if(strcmp(cmd,"help") == 0 ||strcmp(cmd,"?") == 0)
				displayHelp();
			else if (strcmp(cmd, "quit") == 0||strcmp(cmd, "exit") == 0)
				quitCmd(&sock, &sockActif, &sockPassif, reponseServ);
			else
				printf("Unknown command.\n");
		}
	}
	close(sockActif);
	close(sockPassif);
	close(sock);
	return 1;
}

void displayPrompt(char *cmd){
	printf("ftp> ");
	fgets(cmd, BUFSIZ, stdin);
	cmd[strlen(cmd) - 1] = '\0';
}

void argsManagement(int *argc, char *arg, u_short *port){
	if(*argc > 2){
		usage();
		exit(-1);
	}
	else if(*argc == 2){
		if(atoi(arg) != 0)
			*port = (u_short)atoi(arg);
		else{
			printf("Bad port\n");
			usage();
			exit(-1);
		}
	}
	else
		*port = P;
}
void usage(){
	printf("Usage: ./clientftp [port]\n");
}

void printBanner(){
	printf("\n");
	printf("       #==================================================================#\n");
	printf("       |                             Client FTP                           |\n");
	printf("       |                Pour le projet réseau M1 2017-2018                |\n");
	printf("       #==================================================================#\n");
	printf("\n");
}

void displayHelp(){
	printf("List of available commands : \n");
	printf("	ls        : list of remote directory content\n");
	printf("	cd        : change remote directory\n");
	printf("	system    : get remote running system\n");
	printf("	get       : retrieve remote file\n");
	printf("	put       : send local file to remote server\n");
	printf("	delete    : delete remote file or directory\n");
	printf("	mkdir     : create new remote directory\n");
	printf("	rmdir     : delete remote directory\n");
	printf("	rename    : rename remote file/directory\n");
	printf("	passive   : enter passive mode\n");
	printf("	quit/exit : quit ftp client\n");
	printf("	help/?    : display this help\n");
}

int getCode(char reponseServ[]){
    char response[BUFSIZ];
    strcpy(response,reponseServ);
	char *token = strtok(response, " ");
	return atoi(token);
}

int getRandomPort(){
    return rand()%(65535-1024) +1024;
}

int creerSockListen(u_short port, int *isLocalhostConnection){
	int sock, retour;
	struct sockaddr_in adresse;
	sock = socket(AF_INET,SOCK_STREAM,0);

	if (sock<0) {
		perror ("ERREUR OUVERTURE");
		return(-1);
	}

	adresse.sin_family = AF_INET;
	adresse.sin_port = htons(port);
	if(*isLocalhostConnection)
		adresse.sin_addr.s_addr = inet_addr("127.0.0.1");
	else
		adresse.sin_addr.s_addr = INADDR_ANY;

	retour = bind(sock,(struct sockaddr *)&adresse,sizeof(adresse));

	if (retour<0) {
		perror ("IMPOSSIBLE DE NOMMER LA SOCKET");
		return(-1);
	}
	return (sock);
}

int getPasvPort(char reponseServ[]){
	int i = 0, a = 0, b = 0;
	char response[BUFSIZ];
    strcpy(response,reponseServ);
	char *token = strtok(response, "(");
	
	token = strtok(NULL, "(");
	token = strtok(token,")");
	token = strtok(token,",");

	for(i = 0; i < 4 ; i++)
		token = strtok(NULL,",");
		
	a = atoi(token);
	token = strtok(NULL,",");
	b = atoi(token);
	return a*256+b;
}

void getPasvAddr(char *reponseServ, char *adresseDistante){
	int i = 0;
	char *token = strtok(reponseServ, "(");
	token = strtok(NULL, "(");
	token = strtok(token,",");
	adresseDistante[0] = '\0';
	
	for(i = 0 ; i < 4 ; i++){
		strcat(adresseDistante, token);
		if(i < 3)
			strcat(adresseDistante, ".");
		token = strtok(NULL,",");
	}
	
}

int cmdNeedTwoChannels(char cmd[]){
	if(strcmp(cmd,"ls") == 0 || strcmp(cmd,"get") == 0 || strcmp(cmd,"put") == 0)
		return 1;
	return 0;
}

int getNbArgsCmd(char cmd[]){
	int i = 0;
	char cmdBuf[BUFSIZ];
    strcpy(cmdBuf,cmd);
	char *token = strtok(cmdBuf, " ");
	while(token != NULL){
		token = strtok(NULL," ");
		i++;
	}
	return i;
}

void getUserCmd(char *cmd, char *argsCmd){
	char argsCmdBuf[BUFSIZ] = "\0";
	char * token = strtok(cmd," ");
	strcpy(cmd,token);
	token = strtok(NULL," ");
	while(token != NULL){
		strcat(argsCmdBuf,token);
		strcat(argsCmdBuf," ");
		token = strtok(NULL," ");
	}
	strcpy(argsCmd,argsCmdBuf);
}

void getLocalAddressForActiveRequest(char *address, int *isLocalhostConnection){
	struct ifaddrs *addrs, *tmp;
	int i = 0;
	getifaddrs(&addrs);
	tmp = addrs;

	while (tmp){
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET){
			if(*isLocalhostConnection){
				if(strcmp(tmp->ifa_name,"lo") == 0){
					struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
					strcpy(address, inet_ntoa(pAddr->sin_addr));
				}
			}
			else{
				if(strcmp(tmp->ifa_name,"lo") != 0){
					struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
					strcpy(address, inet_ntoa(pAddr->sin_addr));
				}
			}
		}
		tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs); 
	for(i = 0 ; i < strlen(address) ; i++)
		if(address[i] == '.')
			address[i] = ',';
}

void connectToFtpServ(int *sock, struct hostent *recup, struct sockaddr_in adresse, u_short *port, char *reponseServ, int *isLocalhostConnection){
	int s = 0;
	char nomDistant[BUFSIZ];
	adresse.sin_family = AF_INET;
	adresse.sin_port = htons(*port);	
	*sock = socket(AF_INET, SOCK_STREAM, 0);

	if (*sock < 0) {
		perror("Erreur ouverture");
		exit(-1);
	}
	do{
		printf("Adresse serveur : ");
		fgets(nomDistant, BUFSIZ, stdin);
		nomDistant[strlen(nomDistant) - 1] = '\0';
		recup = gethostbyname(nomDistant);
		if (recup == NULL)
			printf("Erreur obtention adresse.\n");
		else{
			memcpy((char *)&adresse.sin_addr, (char *)recup->h_addr, recup->h_length);
			printf("Connexion à l'adresse %s au port %d en cours...\n", inet_ntoa(adresse.sin_addr), *port);
			s = connect(*sock, (struct sockaddr *)&adresse, sizeof(adresse));
			if (s == -1)
				perror("Erreur connexion ");
		}
	}while(recup == NULL || s == -1);
	if(strcmp(inet_ntoa(adresse.sin_addr),"127.0.0.1") == 0)
		*isLocalhostConnection = 1;
	s = read(*sock, reponseServ, 1024);
	if (s == -1) {
		perror("Erreur read");
		exit(-1);
	}
	if(getCode(reponseServ) != 220){
		perror("erreur serveur");
		exit(-1);
	}
	else
		printf("%s",reponseServ);
}

void authentificationFtpServ(int *sock, char *reponseServ){
	int s = 0;
	char reqUser[BUFSIZ]="USER ", reqPass[BUFSIZ]="PASS ", user[BUFSIZ], pass[BUFSIZ];
	
	printf("Nom d'utilisateur : ");
	fgets(user, BUFSIZ, stdin);
	user[strlen(user) - 1] = '\0';
	strcat(reqUser, user);
	strcat(reqUser,"\r\n");
	if (write(*sock, reqUser, strlen(reqUser)) == -1) {
		perror("Erreur ecriture");
		exit(-1);
	}
	else{
		s = read(*sock, reponseServ, 1024);
		reponseServ[s]='\0';
		if(getCode(reponseServ) != 331){
			if(getCode(reponseServ) == 530)
				printf("This server is anonymous only\n");
			exit(-1);
		}
		else{
		    printf("%s",reponseServ);
			printf("Mot de passe : ");
			fgets(pass, BUFSIZ, stdin);
			pass[strlen(pass) - 1] = '\0';
			strcat(reqPass, pass);
			strcat(reqPass,"\r\n");
			if (write(*sock, reqPass, strlen(reqPass)) == -1) {
				perror("Erreur ecriture");
				exit(-1);
			}
			else{
				s = read(*sock, reponseServ, 1024);
				reponseServ[s]='\0';

				if(getCode(reponseServ) != 230){
					if(getCode(reponseServ) == 530)
						printf("Login incorrect.\n");
					exit(-1);
				}
				else{
					printf("%s",reponseServ);
					printf("Authentification ftp reussie\n");
				}
			}
		}			
	}
}

void passiveConnection(int *sock, int *sockPassif, char *reponseServ, struct sockaddr_in adressePassif){
	int s = 0;
	char adresseDistante[BUFSIZ], reqPasv[BUFSIZ]="PASV\r\n";
	struct hostent *adresseDistanteStruct = NULL;
	
	if (write(*sock, reqPasv, strlen(reqPasv)) == -1) {
		perror("Erreur ecriture");
		exit(-1);
	}
	else{
		s = read(*sock, reponseServ, 1024);
		reponseServ[s]='\0';
		if(getCode(reponseServ) != 227){
			perror("erreur serveur");
			exit(-1);
		}
		else{
			printf("%s",reponseServ);
			*sockPassif = socket(AF_INET, SOCK_STREAM, 0);
			if (*sockPassif < 0) {
				perror("Erreur ouverture");
				exit(-1);
			}
			adressePassif.sin_family = AF_INET;
			adressePassif.sin_port = htons(getPasvPort(reponseServ));
			getPasvAddr(reponseServ, adresseDistante);			
			adresseDistanteStruct = gethostbyname(adresseDistante);
			
			memcpy((char *)&adressePassif.sin_addr, (char *)adresseDistanteStruct->h_addr, adresseDistanteStruct->h_length);
		}
	}
	if (connect(*sockPassif, (struct sockaddr *)&adressePassif, sizeof(adressePassif)) == -1) {
		perror("Erreur connexion");
		exit(-1);
	}
}

void preparingActiveMode(int *sockActif, int *sock, char *reponseServ, int *isLocalhostConnection){
	char portBuff[BUFSIZ], reqPort[BUFSIZ], localAddress[BUFSIZ];
	int s = 0, portRandom;
	getLocalAddressForActiveRequest(localAddress, isLocalhostConnection);
	
	portRandom = getRandomPort();
	*sockActif = creerSockListen(portRandom, isLocalhostConnection);
	listen(*sockActif,1);
	
	strcpy(reqPort,"PORT \0");
	sprintf(portBuff, "%d", portRandom / 256);
	strcat(reqPort,localAddress);
	strcat(reqPort,",");
	strcat(reqPort,portBuff);
	strcat(reqPort,",");
	sprintf(portBuff, "%d", portRandom % 256);
	strcat(reqPort,portBuff);
	strcat(reqPort,"\r\n");
	
	if (write(*sock, reqPort, strlen(reqPort)) == -1) {
		perror("Erreur ecriture");
		exit(-1);
	}
	else{
		s = read(*sock, reponseServ, 1024);
		reponseServ[s]='\0';

		if(getCode(reponseServ) != 200){
			perror("erreur serveur");
			exit(-1);
		}
		else
		   printf("%s",reponseServ);
	}
}

void lsCmd(int *sock, int *sockActif, int *sockPassif, int *passiveMode, char *reponseServ){
	int s = 0, sockActifListen = *sockActif;
	char reqList[BUFSIZ] = "LIST\r\n";
	if (write(*sock, reqList, strlen(reqList)) == -1) {
		perror("Erreur ecriture");
		exit(-1);
	}
	else{
		if(!*passiveMode)
			*sockActif = accept(*sockActif, (struct sockaddr *) 0, (unsigned int*) 0);
		s = read(*sock, reponseServ, 1024);
		reponseServ[s]='\0';

		if(getCode(reponseServ) != 150){
			perror("erreur serveur");
			exit(-1);
		}
	
		printf("%s",reponseServ);
	
		if(*passiveMode){
			while((s = read(*sockPassif, reponseServ, 1024)) > 0){
				reponseServ[s]='\0';
				printf("%s",reponseServ);
			}
		}
		else{				
			while((s = read(*sockActif, reponseServ, 1024)) > 0){
				reponseServ[s]='\0';
				printf("%s",reponseServ);
			}
		}
		s = read(*sock, reponseServ, 1024);
		reponseServ[s]='\0';

		if(getCode(reponseServ) != 226){
			perror("erreur serveur");
			exit(-1);
		}
		else
			printf("%s",reponseServ);
	}
	if(*passiveMode){
		close(*sockPassif);
		*sockPassif = -1;
	}
	else{
		shutdown(*sockActif, SHUT_RDWR);
		close(sockActifListen);
	}
}

void cdCmd(int * sock, char *argsCmd, char *reponseServ){
	int s = 0;
	char reqCwd[BUFSIZ];
	if(getNbArgsCmd(argsCmd) != 1)
		printf("Erreurs d'arguments\n");
	else{
		strcpy(argsCmd,strtok(argsCmd," "));
		strcpy(reqCwd,"CWD \0");
		strcat(reqCwd, argsCmd);
		strcat(reqCwd,"\r\n");
		if (write(*sock, reqCwd, strlen(reqCwd)) == -1) {
			perror("Erreur ecriture");
			exit(-1);
		}
		else{
			s = read(*sock, reponseServ, 1024);
			reponseServ[s]='\0';
			if(getCode(reponseServ) != 250){
				if(getCode(reponseServ) == 550)
					printf("Failed to change directory\n");
				else{
					perror("erreur serveur");
					exit(-1);
				}
			}
			else
			   printf("%s",reponseServ);
		}
	}
}

void deleteCmd(int *sock, char *argsCmd, char *reponseServ){
	int s = 0;
	char reqDel[BUFSIZ];
	if(getNbArgsCmd(argsCmd) != 1)
		printf("Erreurs d'arguments\n");
	else{
		strcpy(argsCmd,strtok(argsCmd," "));
		strcpy(reqDel,"DELE \0");
		strcat(reqDel, argsCmd);
		strcat(reqDel,"\r\n");
		if (write(*sock, reqDel, strlen(reqDel)) == -1) {
			perror("Erreur ecriture");
			exit(-1);
		}
		else{
			s = read(*sock, reponseServ, 1024);
			reponseServ[s]='\0';
			if(getCode(reponseServ) != 250){
				if(getCode(reponseServ) == 550)
					printf("File suppression impossible\n");
				else{
					perror("erreur serveur");
					exit(-1);
				}
			}
			else
			   printf("%s",reponseServ);
		}
	}	
}

void mkdirCmd(int *sock, char *argsCmd, char *reponseServ){
	int s = 0;
	char reqMkdir[BUFSIZ];
	if(getNbArgsCmd(argsCmd) != 1)
		printf("Erreurs d'arguments\n");
	else{
		strcpy(argsCmd,strtok(argsCmd," "));
		strcpy(reqMkdir,"MKD \0");
		strcat(reqMkdir, argsCmd);
		strcat(reqMkdir,"\r\n");
		if (write(*sock, reqMkdir, strlen(reqMkdir)) == -1) {
			perror("Erreur ecriture");
			exit(-1);
		}
		else{
			s = read(*sock, reponseServ, 1024);
			reponseServ[s]='\0';
			if(getCode(reponseServ) != 257){
				if(getCode(reponseServ) == 550)
					printf("Directory creation impossible\n");
				else{
					perror("erreur serveur");
					exit(-1);
				}
			}
			else
			   printf("%s",reponseServ);
		}
	}
}

void rmdirCmd(int *sock, char *argsCmd, char *reponseServ){
	int s = 0;
	char reqRmdir[BUFSIZ];
	if(getNbArgsCmd(argsCmd) != 1)
		printf("Erreurs d'arguments\n");
	else{
		strcpy(argsCmd,strtok(argsCmd," "));
		strcpy(reqRmdir,"RMD \0");
		strcat(reqRmdir, argsCmd);
		strcat(reqRmdir,"\r\n");
		if (write(*sock, reqRmdir, strlen(reqRmdir)) == -1) {
			perror("Erreur ecriture");
			exit(-1);
		}
		else{
			s = read(*sock, reponseServ, 1024);
			reponseServ[s]='\0';
			if(getCode(reponseServ) != 250){
				if(getCode(reponseServ) == 550)
					printf("Directory suppression impossible\n");
				else{
					perror("erreur serveur");
					exit(-1);
				}
			}
			else
			   printf("%s",reponseServ);
		}
	}
}

void renameCmd(int *sock, char *argsCmd, char *reponseServ){
	int s = 0;
	char reqRnfr[BUFSIZ], reqRnto[BUFSIZ];
	if(getNbArgsCmd(argsCmd) != 2)
		printf("Erreurs d'arguments\n");
	else{
		strcpy(argsCmd,strtok(argsCmd," "));
		strcpy(reqRnfr,"RNFR \0");
		strcat(reqRnfr, argsCmd);
		strcat(reqRnfr,"\r\n");
	
		strcpy(argsCmd,strtok(NULL," "));
		strcpy(reqRnto,"RNTO \0");
		strcat(reqRnto, argsCmd);
		strcat(reqRnto,"\r\n");
			
		if (write(*sock, reqRnfr, strlen(reqRnfr)) == -1) {
			perror("Erreur ecriture");
			exit(-1);
		}
		else{
			s = read(*sock, reponseServ, 1024);
			reponseServ[s]='\0';
			if(getCode(reponseServ) != 350){
				if(getCode(reponseServ) == 550){
					printf("RNFR command failed\n");
				}
				else{
					perror("erreur serveur");
					exit(-1);
				}
			}
			else{
			   printf("%s",reponseServ);
				if (write(*sock, reqRnto, strlen(reqRnto)) == -1) {
					perror("Erreur ecriture");
					exit(-1);
				}
				else{
					s = read(*sock, reponseServ, 1024);
					reponseServ[s]='\0';
					if(getCode(reponseServ) != 250){
						if(getCode(reponseServ) == 550){
							printf("Rename failed\n");
						}
						else{
							perror("erreur serveur");
							exit(-1);
						}
					}
					else
					   printf("%s",reponseServ);
				}
			}
		}
	}
}

void sendTypeBin(int *isTypeBinSent, int *sock, char *reponseServ){
	int s = 0;
	char reqModeBin[BUFSIZ]="TYPE I\r\n";
	if (write(*sock, reqModeBin, strlen(reqModeBin)) == -1) {
		perror("Erreur ecriture");
		exit(-1);
	}
	else{
		s = read(*sock, reponseServ, 1024);
		reponseServ[s]='\0';
		if(getCode(reponseServ) != 200){
			perror("erreur serveur");
			exit(-1);
		}
		else
		   printf("%s",reponseServ);
	}
	*isTypeBinSent = 1;
}

void getCmd(int *sock, int *sockActif, int *sockPassif, int *isTypeBinSent, char *reponseServ, int *passiveMode, char *argsCmd){
	int s = 0, sockActifListen = *sockActif;
	char reqRetr[BUFSIZ];
	FILE * fichierRecu = NULL;
	if(!isTypeBinSent)
		sendTypeBin(isTypeBinSent, sock, reponseServ);
	if(getNbArgsCmd(argsCmd) != 1)
		printf("Erreurs d'arguments\n");
	else{
		strcpy(argsCmd,strtok(argsCmd," "));
		strcpy(reqRetr,"RETR \0");
		strcat(reqRetr, argsCmd);
		strcat(reqRetr,"\r\n");
		if (write(*sock, reqRetr, strlen(reqRetr)) == -1){
			perror("Erreur ecriture");
			exit(-1);
		}
		else{
			s = read(*sock, reponseServ, 1024);
			reponseServ[s]='\0';
			if(getCode(reponseServ) != 150){
				if(getCode(reponseServ) == 550)
					printf("Failed to open remote file\n");
				else{
					perror("erreur serveur");
					exit(-1);
				}
			}
			else{
				if(!*passiveMode)
					*sockActif = accept(*sockActif, (struct sockaddr *) 0, (unsigned int*) 0);
				printf("%s",reponseServ);
				fichierRecu = fopen(argsCmd,"w");
				if(fichierRecu == NULL){
					perror("Erreur creation fichier");
					exit(-1);
				}
				if(*passiveMode){
					while((s = read(*sockPassif, reponseServ, 1024)) > 0){
						reponseServ[s]='\0';
						fwrite(reponseServ,sizeof(char), s, fichierRecu);
					}
				}
				else{
					while((s = read(*sockActif, reponseServ, 1024)) > 0){
						reponseServ[s]='\0';
						fwrite(reponseServ,sizeof(char), s, fichierRecu);
					}
				}
				fclose(fichierRecu);
				s = read(*sock, reponseServ, 1024);
				reponseServ[s]='\0';
				if(getCode(reponseServ) != 226){
					perror("erreur serveur");
					exit(-1);
				}
				else
				   printf("%s",reponseServ);
			}
		}
	}
	if(*passiveMode){
		close(*sockPassif);
		*sockPassif = -1;
	}
	else{
		shutdown(*sockActif, SHUT_RDWR);
		close(sockActifListen);
	}
}

int getFileSize(FILE * f){
	int size = 0;
	fseek(f, 0, SEEK_END);
	size = ftell(f); 
	fseek(f, 0, SEEK_SET);
	return size;
}

void putCmd(int *sock, int *sockActif, int *sockPassif, int *isTypeBinSent, char *reponseServ, int *passiveMode, char *argsCmd){
	int s = 0, fileSize = 0, sentBytes = 0, writtenBytes = 0, bytesToWrite = 0, sockActifListen = *sockActif;
	char reqStor[BUFSIZ], envoiFichierBuf[BUFSIZ];
	FILE *fichierEnvoi = NULL;
	if(!*isTypeBinSent)
		sendTypeBin(isTypeBinSent, sock, reponseServ);
	if(getNbArgsCmd(argsCmd) != 1)
		printf("Erreurs d'arguments\n");
	else{
		strcpy(argsCmd,strtok(argsCmd," "));
		strcpy(reqStor,"STOR \0");
		strcat(reqStor, argsCmd);
		strcat(reqStor,"\r\n");
		if (write(*sock, reqStor, strlen(reqStor)) == -1) {
			perror("Erreur ecriture");
			exit(-1);
		}
		else{
			s = read(*sock, reponseServ, 1024);
			reponseServ[s]='\0';
			if(getCode(reponseServ)!=150){
				if(getCode(reponseServ) == 553)
					printf("Sending file impossible\n");
				else{
					perror("erreur serveur");
					exit(-1);
				}
			}
			else{
				if(!*passiveMode)
					*sockActif = accept (*sockActif, (struct sockaddr *) 0, (unsigned int*) 0);
				fichierEnvoi = fopen(argsCmd,"rb");
				if(fichierEnvoi == NULL){
					perror("Erreur ouverture fichier");
					exit(-1);
				}
				else{
					fileSize = getFileSize(fichierEnvoi);
					if(*passiveMode){
						while(writtenBytes != fileSize){
							bytesToWrite = ((fileSize - writtenBytes) > BUFSIZ) ? BUFSIZ : (fileSize - writtenBytes);
							bzero(envoiFichierBuf, bytesToWrite);
							fread(envoiFichierBuf,sizeof(char), bytesToWrite,fichierEnvoi);
							if((sentBytes = write(*sockPassif,envoiFichierBuf, bytesToWrite)) == -1){
								perror("Erreur ecriture");
								exit(-1);
							}
				 			writtenBytes += sentBytes;
						}
						close(*sockPassif);
					}
					else{
						while(writtenBytes != fileSize){
							bytesToWrite = ((fileSize - writtenBytes) > BUFSIZ) ? BUFSIZ : (fileSize - writtenBytes);
							bzero(envoiFichierBuf, bytesToWrite);
							fread(envoiFichierBuf,sizeof(char), bytesToWrite,fichierEnvoi);
							if((sentBytes = write(*sockActif,envoiFichierBuf, bytesToWrite)) == -1){
								perror("Erreur ecriture");
								exit(-1);
							}
				 			writtenBytes += sentBytes;
						}
						close(*sockActif);
					}
				  
					fclose(fichierEnvoi);
					s = read(*sock, reponseServ, 1024);
					reponseServ[s]='\0';
					if(getCode(reponseServ) != 226){
						perror("erreur serveur");
						exit(-1);
					}
					else
					   printf("%s",reponseServ);
				}
			}
		}
	}
	if(*passiveMode){
		close(*sockPassif);
		*sockPassif = -1;
	}
	else{
		shutdown(*sockActif, SHUT_RDWR);
		close(sockActifListen);
	}
}

void systemCmd(int *sock, char *reponseServ){
	int s = 0;
	char reqSys[BUFSIZ]="SYST\r\n";
	if (write(*sock, reqSys, strlen(reqSys)) == -1) {
		perror("Erreur ecriture");
		exit(-1);
	}
	else{
	    s = read(*sock, reponseServ, 1024);
		reponseServ[s]='\0';
		if(getCode(reponseServ) != 215){
			perror("erreur serveur");
			exit(-1);
		}
		else
		   printf("%s",reponseServ);
	}
}

void passiveCmd(int *passiveMode){
	if(*passiveMode){
		*passiveMode = 0;
		printf("Passive mode off\n");
	}
	else{
		*passiveMode = 1;
		printf("Passive mode on\n");
	}
}

void quitCmd(int *sock, int *sockActif, int *sockPassif, char *reponseServ){
	int s = 0;
	char reqQuit[BUFSIZ]="QUIT\r\n";
	if (write(*sock, reqQuit, strlen(reqQuit)) == -1){
		perror("Erreur ecriture");
		exit(-1);
	}
	else{
	    s = read(*sock, reponseServ, 1024);
		reponseServ[s]='\0';
		if(getCode(reponseServ) != 221){
			perror("erreur serveur");
			exit(-1);
		}
		else
		   printf("%s",reponseServ);
	}
    close(*sockActif);
    close(*sockPassif);
 	close(*sock);
 	exit(EXIT_SUCCESS);
}
