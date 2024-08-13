#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include <sys/msg.h>
#include "DataStructures.h"


int main()
{
  
  Cleanup cUp;
  cUp.mtype = 11;
  
  char uInput;
  
  int msgid; 
  
  
  key_t key;
  
  if((key = ftok("airtrafficcontroller.c",'A'))==-1)
  {
    printf("Error in generating key!!!!!");
    exit(0);
  }
  
  if((msgid = msgget(key,0666|IPC_CREAT)) == -1)
  {
    printf("\nmsgget Error!!!!!\n");
    exit(0);
  }
  
  printf("\nmsgid: %d",msgid);
  
  cUp.initTermination = false;
  
  
   if(msgsnd(msgid,(void*)&cUp,sizeof(bool),0) == -1)
    {
     printf("\nTermination Failed...\n");
     exit(0);
    }
  
   do{ 
       printf("\nDo you want the Air Traffic Control System to terminate?(Y for Yes and N for No): ");
       scanf(" %c",&uInput);
  }while(uInput!='Y');
  
  cUp.initTermination = true;
  
   
  if(msgsnd(msgid,(void*)&cUp,sizeof(bool),0) == -1)
  {
    printf("\nTermination Failed...\n");
    exit(0);
  }
  
  return 0;
}
