

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pwd.h>

#define M 100
#define N 100
#define BUFF_SIZE 1500



extern void RunCmd();

extern char ipcmd[100];
extern char opcmd[10000];
extern int  status;
char BUF[1000];
//Create a Socket for server communication
short SocketCreate(void)
{
	short hSocket;
	printf("Create the socket\n");
	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	return hSocket;
}
//try to connect with server
int SocketConnect(int hSocket)
{
	int iRetval=-1;
	int ServerPort = 90190;
	struct sockaddr_in remote= {0};
	remote.sin_addr.s_addr = inet_addr("127.0.0.1"); //Local Host
	remote.sin_family = AF_INET;
	remote.sin_port = htons(ServerPort);
	iRetval = connect(hSocket,(struct sockaddr *)&remote,sizeof(struct sockaddr_in));
	return iRetval;
}
// Send the data to the server and set the timeout of 20 seconds
int SocketSend(int hSocket,char* Rqst,short lenRqst)
{
	int shortRetval = -1;
	struct timeval tv;
	tv.tv_sec = 20;  /* 20 Secs Timeout */
	tv.tv_usec = 0;
	if(setsockopt(hSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
	{
		perror("Time Out\n");
		return -1;
	}
	shortRetval = send(hSocket, Rqst, lenRqst, 0);
	return shortRetval;
}
//receive the data from the server
int SocketReceive(int hSocket,char* Rsp,short RvcSize)
{
	int shortRetval = -1;
	struct timeval tv;
	tv.tv_sec = 20;  /* 20 Secs Timeout */
	tv.tv_usec = 0;
	if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0)
	{
		perror("Time Out\n");
		return -1;
	}
	shortRetval = recv(hSocket, Rsp, RvcSize, 0);
	//printf("Response %s\n",Rsp);
	return shortRetval;
}


int hSocket, read_size;

int ftpclientconnecttoserver(int argc, char *argv[])
{

	struct sockaddr_in server;
	hSocket = SocketCreate();
	if(hSocket == -1)
	{
		perror("Socket Creation Failed\n");
		return 1;
	}
	printf("Socket Created\n");

	if (SocketConnect(hSocket) != 0)  //Connect to remote server
	{
		perror("Connection Failed.\n");
		return -1;
	}
	printf("Connection Established with Server Successfully\n");

	return 0;

}
// File transfer
void ftpclientsendtoserver(char incmd[1000],char incmdparts[M][N])
{
	char server_reply[BUFF_SIZE];
	bzero(server_reply, BUFF_SIZE);

	if(!strcmp("get",incmdparts[0])) // receiving a file from the server
	{

		SocketSend(hSocket, incmd, BUFF_SIZE); //Send data to the server

		FILE *fp = fopen(incmdparts[1],"w+");  //Received the data from the server
		if ( fp == NULL )
		{
			printf("\nFile Open Error\n");
		}
		else
		{
			do
			{
				memset(server_reply,'\0',BUFF_SIZE);
				read_size = SocketReceive(hSocket, server_reply, BUFF_SIZE);
				sleep(1);
				fprintf(fp,"%s",server_reply);
				printf("Server Response : %s\n\n",server_reply);

			}while(read_size != -1);


			fclose(fp);
		}
	}
	else if(!strcmp("put",incmdparts[0])) // uploading file to the server
	{

		//Send data to the server
		SocketSend(hSocket, incmd, strlen(incmd));
		sleep(1);

		FILE *fp = NULL;
		if((fp = fopen(incmdparts[1],"r+"))== NULL)
		{
			printf("File Open Error\n");
		}
		else
		{
			do{
				memset(server_reply,'\0',BUFF_SIZE);
				fgets(server_reply,BUFF_SIZE,fp);
				read_size = SocketSend(hSocket, server_reply, BUFF_SIZE);
				sleep(1);
				printf("Transmitted to Server : %s\n\n",server_reply);
			}while(!feof(fp));
			fclose(fp);
		}

	}else{
		//Send data to the server
		SocketSend(hSocket, incmd, strlen(incmd));
		//Received the data from the server
		sleep(1);
		read_size = SocketReceive(hSocket, server_reply, BUFF_SIZE);
		sleep(1);
		printf("Server Response : %s\n\n",server_reply);
	}
	return;
}

void ftpservedisconnect()
{

	char incmd[100];
	memset(incmd,0,100);
	memcpy(incmd,"close",sizeof("close"));
	//Send data to the server
	SocketSend(hSocket, incmd, strlen(incmd));

	sleep(1);

	close(hSocket);
	shutdown(hSocket,0);
	shutdown(hSocket,1);
	shutdown(hSocket,2);
	printf("Connection Closed... Exiting... \n\n");

	return EXIT_SUCCESS;
}

void callpwd()
{
	char buff[500];
	memset(buff,0,sizeof(buff));
	getcwd(buff,sizeof(buff));
	puts(buff);

	return;
}
void call_lcd(char incmdparts[M][N],int cmdcnt)
{

	if(cmdcnt < 1) {
		printf("No cmd lcd\n");
	} else if (cmdcnt == 1) {


		const char *homedir;
		if ((homedir = getenv("HOME")) == NULL)
		{
			homedir = getpwuid(getuid())->pw_dir;
		}
		int ret = chdir(homedir);

		if(ret == -1){
			printf("\nerror in changedir");
		}
	}

	else
	{
		if(strcmp(incmdparts[1], "..") == 0)
		{
			char buff[500];
			memset(buff,0,sizeof(buff));
			getcwd(buff,sizeof(buff));

			for(int i = strlen(buff)-1 ; i>0 ;--i)
			{
				if(buff[i] == '\/')
				{
					buff[i] = '\0';
					break;
				}
			}

			int ret = chdir(buff);
			if(ret == -1){
				printf("\nError in changedir");
			}


			puts("\nDirectory Changed to ");
			puts(buff);

		}
		else
		{

			int ret = chdir(incmdparts[1]);
			if(ret == -1){
				printf("\nerror in changedir");
			}

		}
	}

	return;
}
void call_lls(char incmdparts[M][N], int cmdcnt)
{

	struct dirent **namelist;
	int n=0;
	if(cmdcnt < 1) {
		printf("No cmd lls\n");
	} else if (cmdcnt == 1) {
		char buff[500];
		memset(buff,0,sizeof(buff));
		getcwd(buff,sizeof(buff));
		n=scandir(buff,&namelist,NULL,alphasort);
	} else {
		n = scandir(incmdparts[1], &namelist, NULL, alphasort);
	}
	if(n < 0) {
		printf("Error in scan dircmdcnt=%d, n=%d\n",cmdcnt,n);
	} else {
		while (n--) {
			printf("%s\n",namelist[n]->d_name);
			free(namelist[n]);
		}
		free(namelist);
	}

	return;
}
void call_lchmod(char incmdparts[M][N], int cmdcnt)
{

	if(cmdcnt < 3) {
		printf("Correct no. of argument not passed\n");
	}
	else if (cmdcnt == 3)
	{


		FILE *fp = fopen(incmdparts[2],"r");
		if(fp == NULL)
		{
			printf("\n File/Directory doesn't exist\n");
		}
		else
		{
			int i = strtol(incmdparts[1],0,8);
			int ret = chmod(incmdparts[2],i);
			if (ret == 0)
			{
				printf("\n Permission changed successfully!!!\n");

			}
			else
				perror(ret);

			fclose(fp);
		}

	}

}




int mymain(int argc, char *argv[]) {


	ftpclientconnecttoserver(argc,argv);
	printf("\033[2J\033[0;0H");
	printf("--------------------- FTP CLIENT SHELL OS ELL 783 GROUP 6----------------------\n");

	int maxcmds=9;
	char * cmdlist[]={
			"ls",
			"cd",
			"chmod",
			"lls",
			"lcd",
			"lchmod",
			"put",
			"get",
			"close"};



	while(1){
		char incmd[1000];
		char incmd1[1000];
		char incmdparts[M][N];
		memset(incmd,0,sizeof(incmd));
		memset(incmdparts,0,sizeof(incmdparts));
		printf("\n$");
		fflush(stdin);
		gets(incmd);
		memcpy(incmd1,incmd,1000);
		char * token=NULL, *saveptr1=NULL, *str1=incmd;
		int count=0;
		for (count = 0 ; ; count++, str1 = NULL) {
			token = strtok_r(str1,(char *)" ", &saveptr1);
			if (token == NULL){

				break;
			}
			strcpy(incmdparts[count],token);
		}


		int cmdfound=-1;
		for(int i=0;i<maxcmds;i++){
			if(strcmp(incmdparts[0],cmdlist[i])==0){
				cmdfound=i;
				break;
			}
		}
		/////
		switch(cmdfound){

		case 0:
		case 1:
		case 2:
		case 6:
		case 7:
			ftpclientsendtoserver( incmd1 ,incmdparts);
			break;
		case 3:
			call_lls(incmdparts,count);
			break;
		case 4:
			call_lcd(incmdparts,count);
			break;
		case 5:
			call_lchmod(incmdparts,count);
			break;
		case 8:
			//close
			// disconnect from the server
			ftpservedisconnect();
			exit(1);
			break;
		default:
		{
			int ret = 0;
			char buffer[1000];
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer,"/bin/%s",incmd);
			ret = fork();
			if(ret ==0){
				execlp(buffer,buffer,NULL);
			}else{
				wait(NULL);
			}
		}
		}
	}

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {

	mymain(argc, argv);
}

