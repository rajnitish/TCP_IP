#ifndef RUNCMD_C
#define RUNCMD_C

#include<string.h>
#include<stdio.h>
char ipcmd[100]={0};
char opcmd[10000]={0};

FILE *fp;
int status;

void RunCmd()
{

char path[4096];
//sprintf(ipcmd,"%s","ls");

fp = popen(ipcmd, "r");
if (fp == NULL){
   printf("Err:cmd Not executed %s",ipcmd);
}


while (fgets(path, sizeof(path), fp) != NULL){
    printf("%s\n", opcmd);
    strcat(opcmd,path);
}
//printf("%s", opcmd);
status = pclose(fp);
 //sleep(1);
 return;
}

#endif
