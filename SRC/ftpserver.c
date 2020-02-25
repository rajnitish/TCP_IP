

#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
extern void RunCmd();

extern char ipcmd[100];
extern char opcmd[10000];
extern int status;

#define M 100
#define N 100


//-----

int  sock;
short SocketCreate(void)
{
    short hSocket;
    printf("Create the socket\n");
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    return hSocket;
}
int BindCreatedSocket(int hSocket)
{
    int iRetval=-1;
    int ClientPort = 90190;
    struct sockaddr_in  remote= {0};
    /* Internet address family */
    remote.sin_family = AF_INET;
    /* Any incoming interface */
    remote.sin_addr.s_addr = htonl(INADDR_ANY);
    remote.sin_port = htons(ClientPort); /* Local port */
    iRetval = bind(hSocket,(struct sockaddr *)&remote,sizeof(remote));
    return iRetval;
}


//-------

char servlocopcmd[100000];

void callpwd()
{
	char buff[500];
	memset(buff,0,sizeof(buff));
	getcwd(buff,sizeof(buff));
	puts(buff);

	return;
}
void call_cd(char incmdparts[M][N])
{
	if(strcmp(incmdparts[1], NULL) == 0){

		int ret = chdir("$(HOME)");
		if(ret == -1){
			printf("\nerror in changedir");
		}
	}else if(strcmp(incmdparts[1], "..") == 0){
		//char buffer[1000];
		//memset(buffer,0,1000);
		//sprintf("%s/%s",incmdparts[1],incmdparts[1])
	} else{

		int ret = chdir(incmdparts[1]);
		if(ret == -1){
			printf("\nerror in changedir");
		}
	}

	return;
}
void call_ls(char incmdparts[M][N], int cmdcnt)
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

	memset(servlocopcmd,0,100000);
	if(n < 0) {
		printf("Error in scan dircmdcnt=%d, n=%d\n",cmdcnt,n);
	} else {
		while (n--) {

			printf("%s\n",namelist[n]->d_name);
			strcat(servlocopcmd,namelist[n]->d_name);
			free(namelist[n]);
		}
		free(namelist);
	}
	// send data to client
	if( send(sock, servlocopcmd, strlen(servlocopcmd), 0) < 0)
	{
		printf("Send failed");
	}
	memset(servlocopcmd,0,100000);
	return;
}


//------
int main(int argc, char *argv[])
{
    int socket_desc, clientLen, read_size;
    struct sockaddr_in server, client;
    char client_message[200]= {0};
    char message[100] = {0};
    const char *pMessage = "hello";
    //Create socket
    socket_desc = SocketCreate();
    if (socket_desc == -1)
    {
        printf("Could not create socket");
        return 1;
    }
    printf("Socket created\n");
    //Bind
    if( BindCreatedSocket(socket_desc) < 0)
    {
        //print the error message
        perror("bind failed.");
        return 1;
    }
    printf("bind done\n");
    //Listen
    listen(socket_desc, 3);
    //Accept and incoming connection

    int maxcmds=6;
	char * cmdlist[]={
			"ls",
			"cd",
			"chmod",
			"put",
			"get",
			"close"};

    while(1)
    {
        printf("Waiting for incoming connections...\n");
        clientLen = sizeof(struct sockaddr_in);
        //accept connection from an incoming client
        sock = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&clientLen);
        if (sock < 0)
        {
            perror("accept failed");
            return 1;
        }
        printf("Connection accepted\n");
        memset(client_message, '\0', sizeof client_message);
        memset(message, '\0', sizeof message);
        //Receive a reply from the client
        if( recv(sock, client_message, 200, 0) < 0)
        {
            printf("recv failed");
            break;
        }
        printf("Client reply : %s recieved\n",client_message);

        char incmd[1000];
		char incmdparts[M][N];
		memset(incmd,0,sizeof(incmd));
		memset(incmdparts,0,sizeof(incmdparts));
		sprintf(incmd,"%s",client_message);
		printf("\n$");

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

		/*
        if(strcmp("ls",client_message)==0)
        {
            memset(ipcmd,0,100);
            sprintf(ipcmd,"%s","ls");
            RunCmd();
            strcpy(message,opcmd);
            memset(opcmd,0,10000);
            // Send data
            if( send(sock, message, strlen(message), 0) < 0)
            {
                printf("Send failed");
                return 1;
            }
        } 
        else if(strcmp("cd",client_message)==0)
        {
            memset(ipcmd,0,100);
            sprintf(ipcmd,"%s","cd");
            RunCmd();
            strcpy(message,opcmd);
            memset(opcmd,0,10000);
            // Send data
            if( send(sock, message, strlen(message), 0) < 0)
            {
                printf("Send failed");
                return 1;
            }
        } 
        else if(strcmp("put",client_message)==0)
        {
            // separate put and file name
            // open file, if file not present create file
            //recieve data from client and write to file
        } 
        else if(strcmp("get",client_message)==0)
        {
            // separate put and file name
            // open file, if file not present send error msg to client
            // send data from server to client to write in a file
        } else
        {
            strcpy(message,"Invalid cmd mps !");
        }
        
        */

		switch(cmdfound){

		case 0:
			call_ls(incmdparts,count);
			break;
		case 1:
			call_cd(incmdparts);
			break;
		case 2:
			//call_lchmod
			{
			int i;
			i = atoi(incmdparts[1]);
			if (chmod (incmdparts[3],i) < 0)
				printf("error in chmod");
			}
			break;
		case 4:
			//put
			// send file to server
			break;
		case 5:
			//get
			// get file from server
			break;
		case 6:
			//close
			// disconnect from the server
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
        
        close(sock);
        sleep(1);
    }
    return 0;
}


