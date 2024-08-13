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

//checking unique id in array
bool checkUnique(int* arr,int size,int num)
{
   
   for(int i=0;i<size;i++)
   {
      if(arr[i] == num)
      {
        return true;
      }
   }
   
   return false;
}  

//Adding Port Number(visited/to be visited) in Array...
int StorePortNum(int* arr,int count,int num)
{
   if(count==0)
   {
      arr[count] = num;
      count++;
      return count;
   }
   
   if(!checkUnique(arr,count,num))
   {
      arr[count] = num;
      count++;
   }
   return count;
}

int main()
{
  
  Message mg,Imsg,Fmsg;
  Cleanup Cup,plane_t,port_t;
  
  //message id created: 
  int msgID;
  key_t qKey;  //creating a key unique to filepath....
  
  if((qKey = ftok("airtrafficcontroller.c",'A'))==-1)
  {
    printf("Error in generating key!!!!!");
    exit(0);
  }
  
  if((msgID = msgget(qKey,IPC_CREAT))==-1)
  {
    printf("Messege Queue Not Accessible...");
    exit(0);
  }
  
  plane_t.mtype = 13;
  plane_t.initTermination = false;    //initializing plane.c termination command
  
  msgsnd(msgID,(void*)&port_t,sizeof(bool),0);
  
  msgsnd(msgID,(void*)&plane_t,sizeof(bool),0);
  
  //Enter number of airport to be handled...
  int numAirport;
  printf("\nEnter the number of airports to be handled/managed: ");
  scanf("%d",&numAirport);
  
  //defining array to store portnumber....
  int portNum[numAirport];
  int size =0;
  
  //creating and opening txt file for writing("w") purpose....
  FILE *fptr;
  fptr = fopen("airtrafficcontroller.txt","w");
  
  if(fptr == NULL)
  {
    printf("\n!!!!Error opening file!!!!\n");
    exit(0);
  }
  
  //running while loop until Cleanup command...
  bool terminate = false;
  do
  {
  
   if(msgrcv(msgID,(void*)&Cup,sizeof(bool),11,IPC_NOWAIT)!=-1)  //receiving termination request from cleanup.c
   {
      terminate = Cup.initTermination; 
      
   }
   
   if(!terminate)
   {
     if(msgrcv(msgID,(void*)&mg,sizeof(Flight_Details),12,IPC_NOWAIT)!=-1 && !mg.Fd.sent)
     {
       
       size = StorePortNum(portNum,size,mg.Fd.Departure_Code);    //adding unique port(departure/arrival) to array...
       size = StorePortNum(portNum,size,mg.Fd.Arrival_Code);
       
       Imsg = mg;
       
       Imsg.mtype = mg.Fd.Departure_Code;  
       
       
       msgsnd(msgID,(void*)&Imsg,sizeof(Flight_Details),0);  //sending flight details to departure port...
         
       mg.Fd.sent = true;
       printf("\nPlane_ID: %d, Weight: %d size: %d",mg.Fd.planeID,mg.Fd.Total_Weight,size);
       fflush(stdout);
     }
   }
   
   if(msgrcv(msgID,(void*)&Fmsg,sizeof(Flight_Details),14,IPC_NOWAIT)!=-1)  //receiving message(if any) from airport.c...
        {
           
           //cheking its current stats(departure or arrival)...
          if(Fmsg.Fd.lStats && !Fmsg.Fd.ulStats)  
          {
             //if departed then send message to Arrival port and print details to airtrafficcontroller.txt...
             fprintf(fptr,"Plane %d has departed from Airport %d and will land at Airport %d.\n",Fmsg.Fd.planeID,Fmsg.Fd.Departure_Code,Fmsg.Fd.Arrival_Code);
             Fmsg.mtype = Fmsg.Fd.Arrival_Code;  
             msgsnd(msgID,(void*)&Fmsg,sizeof(Flight_Details),0);
          }
          if(Fmsg.Fd.ulStats)
          {
            //if landed then terminate plane.c...
            plane_t.initTermination = true;
            msgsnd(msgID,(void*)&plane_t,sizeof(bool),0);
          }
        }				 
   
  
 }while(!terminate);
  
  
  fclose(fptr); //closing txt file...
  
  //indvidually sending message to each port for termination...
  port_t.initTermination = true;
  for(int i=0;i<size;i++)
  {
   
   port_t.mtype = 20+portNum[i];
   if(msgsnd(msgID,(void*)&port_t,sizeof(bool),0)==-1)  //sending termination requests to all ports....
   {
     printf("\nmsg nt snd");
   }
  }
  sleep(10);
  if(msgctl(msgID,IPC_RMID,NULL)==-1)   //deleting the message queue...
  { 
    printf("\nDeletion Error");
    exit(0);
  } 
  
return 0;
}
