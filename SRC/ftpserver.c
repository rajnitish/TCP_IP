

#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
#include<pwd.h>
#include<signal.h>

extern void RunCmd();

extern char ipcmd[100];
extern char opcmd[10000];
extern int status;

#define M 100
#define N 100


#define BUFF_SIZE 1500

char servlocopcmd[100000];
int  sock;

short CreateSocket(void)
{
	short hSocket;
	printf("Socket Create Initation\n");
	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	return hSocket;
}
int BindSocket(int hSocket)
{
	int iRetval					=-1;
	int ClientPort 				= 90190;
	struct sockaddr_in  remote	= {0};

	remote.sin_family 			= AF_INET; 					//Internet address family
	remote.sin_addr.s_addr 		= htonl(INADDR_ANY);   		// Any incoming interfaces
	remote.sin_port 			= htons(ClientPort); 		// Local port

	iRetval = bind(hSocket,(struct sockaddr *)&remote,sizeof(remote));
	return iRetval;
}

int SocketSend(char* Rqst,short lenRqst)
{
	int shortRetval = -1;
	struct timeval tv;
	tv.tv_sec = 20;  /* 20 Secs Timeout */
	tv.tv_usec = 0;
	if(setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
	{
		printf("Time Out\n");
		return -1;
	}
	shortRetval = send(sock, Rqst, lenRqst, 0);
	return shortRetval;
}
//receive the data from the server
int SocketReceive(char* Rsp,short RvcSize)
{
	int shortRetval = -1;
	struct timeval tv;
	tv.tv_sec = 20;  /* 20 Secs Timeout */
	tv.tv_usec = 0;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0)
	{
		printf("Time Out\n");
		return -1;
	}
	shortRetval = recv(sock, Rsp, RvcSize, 0);
	//printf("Response %s\n",Rsp);
	return shortRetval;
}


void callpwd()
{
	char buff[500];
	memset(buff,0,sizeof(buff));
	getcwd(buff,sizeof(buff));
	puts(buff);

	return;
}
void call_cd(char incmdparts[M][N],int cmdcnt)
{

	memset(servlocopcmd,0,100000);
	char successful = 0;

	if(cmdcnt < 1) {
		printf("No cmd lcd\n");
	} else if (cmdcnt == 1) {

		memset(ipcmd,0,50);
		memset(opcmd,0,50);
		sprintf(ipcmd,"echo $HOME");
		RunCmd();
		memcpy(opcmd+strlen(opcmd)-1,"\/\0",2);
		puts(" Path changed to ::");
		puts(opcmd);
		int ret = chdir(opcmd);

		if(ret == -1)
		{
			printf("\nError in changedir");
		}
		else
		{
			successful = 1;

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

			puts(" Path changed to ::");
			puts(buff);
			int ret = chdir(buff);
			if(ret == -1){
				printf("\nError in changedir");

			}
			else
			{
				successful = 1;

			}


		}
		else
		{

			int ret = chdir(incmdparts[1]);
			if(ret == -1){
				printf("\nerror in changedir\n");
			}
			else
			{
				successful = 1;
			}

		}

	}

	if(successful)
	{

		char newBuff[500];
		getcwd(newBuff,500);
		strcat(servlocopcmd,"\nDirectory Changed at server Side :: ");
		strcat(servlocopcmd,newBuff);

		if( send(sock, servlocopcmd, strlen(servlocopcmd), 0) < 0)
		{
			perror("Send failed");
		}
	}

	sleep(1);
	memset(servlocopcmd,0,100000);

	return;
}
void call_ls(char incmdparts[M][N], int cmdcnt)
{

	struct dirent **namelist;
	int n=0;
	if(cmdcnt < 1)
	{
		printf("No cmd lls\n");
	}
	else if (cmdcnt == 1)
	{
		char buff[500];
		memset(buff,0,sizeof(buff));
		getcwd(buff,sizeof(buff));
		n=scandir(buff,&namelist,NULL,alphasort);
	}
	else
	{
		n = scandir(incmdparts[1], &namelist, NULL, alphasort);
	}

	memset(servlocopcmd,0,100000);
	if(n < 0)
	{
		printf("Error in scan dircmdcnt=%d, n=%d\n",cmdcnt,n);
	}
	else
	{
		while (n--)
		{

			printf("%s\n",namelist[n]->d_name);
			strcat(servlocopcmd,"\n");
			strcat(servlocopcmd,namelist[n]->d_name);
			free(namelist[n]);
		}
		free(namelist);
	}
	// send data to client
	if( send(sock, servlocopcmd, strlen(servlocopcmd), 0) < 0)
	{
		perror("Send failed");
	}
	memset(servlocopcmd,0,100000);
	return;
}


void serve_put(char incmdparts[M][N], int cmdcnt)
{
	if(cmdcnt < 2)
	{
		printf("Argument of get not passed\n");
	}
	else if (cmdcnt == 2)
	{

		FILE *fp = fopen(incmdparts[1],"a+");
		if ( fp == NULL )
		{
			printf( "\n file failed to open.\n" ) ;
		}
		else
		{
			int read_size;

			do{

				memset(servlocopcmd,'\0',BUFF_SIZE);
				read_size = SocketReceive(servlocopcmd, BUFF_SIZE);
				sleep(1);
				fprintf(fp,"%s",servlocopcmd);
				printf("Write  in file  : %s  \n\n",servlocopcmd);


			}while(read_size != -1);

			fclose(fp);
		}

	}
}

void serve_get(char incmdparts[M][N], int cmdcnt)
{
	if(cmdcnt < 2) {
		printf("Argument of get not passed\n");
	} else if (cmdcnt == 2) {
		char buff[500];
		memset(buff,0,sizeof(buff));

		FILE *fp = fopen(incmdparts[1],"r");
		if ( fp == NULL )
		{
			printf( "\n file failed to open.\n" ) ;
		}
		else
		{
			do{

				memset(servlocopcmd,'\0',BUFF_SIZE);
				fgets(servlocopcmd,BUFF_SIZE,fp);

				if(SocketSend(servlocopcmd, strlen(servlocopcmd)) < 0)
				{
					perror("\nSend failed");
				}

				printf("transmitted to server : %s\n\n",servlocopcmd);
				sleep(1);
			}while(!feof(fp));

			fclose(fp);
		}


	}


}

void serve_chmod(char incmdparts[M][N], int cmdcnt)
{

	memset(servlocopcmd,'\0',BUFF_SIZE);

	if(cmdcnt < 3) {
		printf("Correct no. of argument not passed\n");
		memcpy(servlocopcmd,"Correct no. of argument not passed\0",BUFF_SIZE);
	}
	else if (cmdcnt == 3)
	{


		FILE *fp = fopen(incmdparts[2],"r");
		if(fp == NULL)
		{
			printf("\n File/Directory doesn't exist\n");
			memcpy(servlocopcmd,"File/Directory doesn't exist\0",BUFF_SIZE);
		}
		else
		{
			int i = strtol(incmdparts[1],0,8);
			int ret = chmod(incmdparts[2],i);
			if (ret == 0)
			{
				printf("\n Permission changed!!!\n");

				memcpy(servlocopcmd," Permission changed successfully!!!\0",BUFF_SIZE);
			}
			else
				perror(ret);

			fclose(fp);
		}

	}
	sleep(1);
	if(SocketSend(servlocopcmd, strlen(servlocopcmd)) < 0)
	{
		perror("\nSend failed");
	}

}

void ClientCount(int flag)
{
	int noOfClient;

	memset(ipcmd,0,100);
	memset(opcmd,0,100);
	sprintf(ipcmd,"ps -ax | grep CLIENT | wc -l");
	RunCmd();
	noOfClient = atoi(opcmd) - 2;

	if(flag)
	{
		printf("Client Added !!! The total # of Client currently active is %d\n",noOfClient);
	}
	else
	{
		noOfClient--;
		printf("Client Removed !!! The total # of Client currently active is %d\n",noOfClient);
	}

}


//------
int main(int argc, char *argv[])
{
	int socket_desc, clientLen, read_size;
	struct sockaddr_in server, client;
	char client_message[BUFF_SIZE]= {0};
	const char *pMessage = "hello";


	socket_desc = CreateSocket();  //Create socket
	if (socket_desc == -1)
	{
		perror("Socket Creation Failed");
		return 1;
	}
	printf("Socket Created\n");

	if( BindSocket(socket_desc) < 0)
	{
		perror("Binding Failed");
		return 1;
	}
	printf("Socket Binding Done\n");
	listen(socket_desc, 3); // listen
	//Accept and incoming connection

	int maxcmds=6;
	char * cmdlist[]={
			"ls",
			"cd",
			"chmod",
			"put",
			"get",
			"close"};

	printf("Listening:: Waiting for incoming connections...\n");
	clientLen = sizeof(struct sockaddr_in);

	pid_t pid;

	while (1)
	{
		sock = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&clientLen); //accept connection from an incoming client
		if (sock < 0)
		{
			perror("Connection Refused/Failed");
			exit(0);
		}

		printf("Connection Accepted\n");

		ClientCount(1);

		pid = fork();

		if (pid < 0)
		{
			perror("Error!! Fork failed");
			exit(0);
		}
		if (pid == 0)
		{

			while(1)
			{

				memset(client_message, '\0', sizeof(client_message));

				//Receive a reply from the client
				if( recv(sock, client_message, BUFF_SIZE, 0) < 0)
				{
					perror("Error!! While reading from socket");
					exit(0);
				}
				else
				{

					printf("Client reply :: %s recieved\n",client_message);

					char incmd[1000],incmd1[1000];
					char incmdparts[M][N];
					memset(incmd,0,sizeof(incmd));
					memset(incmdparts,0,sizeof(incmdparts));
					sprintf(incmd,"%s",client_message);
					printf("\n$");


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


					switch(cmdfound){

					case 0:
						call_ls(incmdparts,count);
						break;
					case 1:
						call_cd(incmdparts,count);
						break;

					case 2:
						serve_chmod(incmdparts,count);
						break;
					case 3:
						//put
						// send file to server
						serve_put(incmdparts,count);
						break;
					case 4:
						//get
						// get file from server
						serve_get(incmdparts,count);
						break;
					case 5:
						//close
						// disconnect from the server
						close(sock);
						ClientCount(0);
						kill(getpid(),SIGTERM);


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

					sleep(1);
				}
			}

		}
		else
		{
			close(sock);
		}
	}

	return 0;
}

