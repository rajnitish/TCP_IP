#ifndef FTPSERVER_C
#define FTPSERVER_C

#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
extern void RunCmd();

extern char ipcmd[100];
extern char opcmd[10000];
extern int status;

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
int main(int argc, char *argv[])
{
    int socket_desc, sock, clientLen, read_size;
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
        printf("Client reply : %s\n",client_message);
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
        
        
        
        close(sock);
        sleep(1);
    }
    return 0;
}

#endif
