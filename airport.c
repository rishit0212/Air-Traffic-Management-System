#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <semaphore.h>
#include "DataStructures.h"

//defining runway number for backup runway...
#define BackupRunway -1

sem_t semaphore;  //defining semaphore to be used for synchronization...

/*defining thread method for departure*/
void* departureThread(void* param)
{
  
  t_data dat = *(t_data*)param;
  
  int runway = BackupRunway;
  int temp = dat.backCap;
  
  int weight = dat.msg.Fd.Total_Weight;
  //checking for suitable runway to land...
  for(int i=0;i<dat.size;i++)
  {
     if(weight<=dat.capacity[i])
     {
        if(dat.capacity[i]<temp)
        {
          runway = i+1;
          temp = dat.capacity[i];
        }
     }
        
  }
  sem_wait(&semaphore);
  //sleep(3);
  printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.",dat.msg.Fd.planeID,runway,dat.msg.Fd.Departure_Code);
  fflush(stdout);
  sem_post(&semaphore);
 return NULL; 
}

/*defining thread method for arrival*/
void* arrivalThread(void* param)
{		
   t_data dat = *(t_data*)param;
  
  int runway = BackupRunway;
  int temp = dat.backCap;
  
  int weight = dat.msg.Fd.Total_Weight;
  
  //checking for suitable runway to land...
  for(int i=0;i<dat.size;i++)
  {
     if(weight<=dat.capacity[i])
     {
        if(dat.capacity[i]<temp)
        {
          runway = i+1;
          temp = dat.capacity[i];
        }
     }
        
  }
  
 sem_wait(&semaphore);
 
 printf("\nPlane %d has landed on Runway No. %d of Airport No. %d and completed deboarding/unloading.",dat.msg.Fd.planeID,runway,dat.msg.Fd.Arrival_Code);
  fflush(stdout);
 sem_post(&semaphore);
 
 return NULL; 
   
   
}


int main()
{

  Message message,S_msg;
  t_data dt;
  Cleanup port_t;
  
  int pNum;
  printf("\nEnter Airport Number: ");
  scanf("%d",&pNum); //Range (1-10)
  
  int rNum;
  printf("\nEnter number of Runways: ");
  scanf("%d",&rNum); //range (1-10);
  
  int rCapacity[rNum];
  
  printf("\nEnter loadCapacity of Runways (give as a space separated list in a Single Line):"); //range (1000-12000 kg);
  
  //Enter Weight Capacity for Each Runway
  int i=0;
  do
  {
     scanf("%d",&rCapacity[i++]);
  }while(i!=rNum);
  
 int bCapacity = 15000; //Capacity for Backup Runway.;
 
 key_t key;
 key = ftok("airtrafficcontroller.c",'A');
 if(key == -1)
 {
   printf("\nKey Not Generated....");
   exit(0);
 }
 
 //message queue created or accessed: 
  int msgID;
  msgID = msgget(key,0666|IPC_CREAT);
  if(msgID == -1)
  {
    printf("\nMessege Queue Not Accessible...");
    exit(0);
  }
  printf("\nmsgid: %d",msgID);
  
  pthread_t tid1,tid2;
  
  pthread_attr_t attr;
  
  pthread_attr_init(&attr);
  
  if(sem_init(&semaphore,0,1)==-1)
  {
    printf("\n!!!!Error in creating Semaphore!!!\n");
    exit(0);
  }
  
  bool terminate = false;
  do{
  if(msgrcv(msgID,(void*)&port_t,sizeof(bool),20+pNum,IPC_NOWAIT)!=-1)
  {
     terminate = port_t.initTermination;
    
  }
  
  if(!terminate){
  
  if(msgrcv(msgID,(void*)&message,sizeof(Flight_Details),pNum,IPC_NOWAIT)!=-1){
  S_msg = message;
  
  
      
  printf("\nPlaneID: %d",message.Fd.planeID);
  fflush(stdout);
  //transfering ALL airport data to 't_data' Structure
  dt.msg = message;
  dt.size = rNum;
  dt.capacity = rCapacity;
  dt.backCap = bCapacity;
 
  if(pNum == message.Fd.Departure_Code) //Defining Process for departure port...
  {
   if(!message.Fd.lStats){
    S_msg.Fd.lStats = true;
    pthread_create(&tid1,&attr,departureThread,(void*)&dt);
    pthread_join(tid1,NULL);
    
    S_msg.mtype = 14;
    
    if(msgsnd(msgID,(void*)&S_msg,sizeof(Flight_Details),0)==-1)
    {
      printf("message not sent");
      exit(0);
    }
   
   }
  }
  if(pNum == message.Fd.Arrival_Code)     //Defining Process for arrival port...
  { 
   if(!message.Fd.ulStats && message.Fd.lStats){
    S_msg.Fd.ulStats = true;
    pthread_create(&tid2,&attr,arrivalThread,(void*)&dt);
    pthread_join(tid2,NULL);
    
    S_msg.mtype = 14;
    msgsnd(msgID,(void*)&S_msg,sizeof(Flight_Details),0);
    }
   }
  
  }
  
    
 }
  
 }while(!terminate);
 
 sem_destroy(&semaphore);
 return 0;
}
