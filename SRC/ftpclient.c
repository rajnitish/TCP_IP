

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#define M 100
#define N 100



extern void RunCmd();

extern char ipcmd[100];
extern char opcmd[10000];
extern int status;

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
        printf("Time Out\n");
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
        printf("Time Out\n");
        return -1;
    }
    shortRetval = recv(hSocket, Rsp, RvcSize, 0);
    //printf("Response %s\n",Rsp);
    return shortRetval;
}
//main driver program
int ftpclientmain(int cmd,int argc, char *argv[])
{
    int hSocket, read_size;
    struct sockaddr_in server;
    char SendToServer[100] = {0};
    char server_reply[200] = {0};
    //Create socket
    hSocket = SocketCreate();
    if(hSocket == -1)
    {
        printf("Could not create socket\n");
        return 1;
    }
    printf("Socket is created\n");
    //Connect to remote server
    if (SocketConnect(hSocket) < 0)
    {
        perror("connect failed.\n");
        return 1;
    }
    printf("Sucessfully conected with server\n");
    printf("Enter the Message: ");
    
    gets(SendToServer);
    if(strcmp("lls",SendToServer)==0){
        memset(ipcmd,0,100);
        sprintf(ipcmd,"%s","ls");
        RunCmd();
        puts(opcmd);
        memset(opcmd,0,10000);
        
    } else if(strcmp("lcd",SendToServer)==0){
       // code for cd , cd .. , cd givendir usind chdir()
    } else{
           
        //Send data to the server
        SocketSend(hSocket, SendToServer, strlen(SendToServer));
        //Received the data from the server
        read_size = SocketReceive(hSocket, server_reply, 200);
        printf("Server Response : %s\n\n",server_reply);
        close(hSocket);
        shutdown(hSocket,0);
        shutdown(hSocket,1);
        shutdown(hSocket,2);
    }
    return 0;
}

void callpwd()
{
	char buff[500];
	memset(buff,0,sizeof(buff));
	getcwd(buff,sizeof(buff));
	puts(buff);

	return;
}
void call_lcd(char incmdparts[M][N])
{

	int ret = chdir(incmdparts[1]);
	if(ret == -1){
		printf("\nerror in changedir");
	}

	return;
}
void call_lls(char incmdparts[M][N], int cmdcnt)
{

	struct dirent **namelist;
	int n;
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
		puts(“Error in scan dir”);
	} else {
		while (n--) {
			printf(“%s\n”,namelist[n]->d_name);
			free(namelist[n]);
		}
		free(namelist);
	}

	return;
}
int mymain(int argc, char *argv[]) {
	printf("\033[2J\033[0;0H");
	printf("--------------------- DUMMY SHELL OS ELL 783 GROUP 6----------------------\n");

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
		char incmdparts[M][N];
		memset(incmd,0,sizeof(incmd));
		memset(incmdparts,0,sizeof(incmdparts));
		printf("\n$");
		fflush(stdin);
		gets(incmd);
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

		case 0://path
			//call_ls(incmdparts);
			break;
		case 1://pushd
			//call_cd(incmdparts);
			break;
		case 2://popd
			//call_chmod();
			break;
		case 3://copy
			call_lls(incmdparts,count+1);
			break;
		case 4://cd
			call_lcd(incmdparts);
			break;
		case 5://newshell
			//call_lchmod
			break;
		case 6://exit
			//put

			break;
		case 7://pwd
			//get
			break;
		case 8://pwd
			//close
			break;
		default:
		{
			int ret = 0;
			char buffer[1000];
			memset(buffer,0,sizeof(buffer));
			if(find("/bin") != NULL){
				sprintf(buffer,"/bin/%s",incmdparts[0]);
				ret = fork();
				if(ret ==0){
					execlp(buffer,buffer,NULL);
				}else{
					wait(NULL);
				}
			}
			else

				puts("Invalid command");
		}
		}
	}

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {

	mymain(argc, argv);
}

