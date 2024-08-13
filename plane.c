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



//defining read and write end for the pipe
#define read_end 0
#define write_end 1

//defining average crew weight for both types of planes...
#define avgCrewWgt 75

typedef struct {

  int lWeight;
  int bWeight;
}pDetails;

int main() {

    Message MSG;
    Flight_Details fd;

    int p_id;
    //Enter Plane ID (1-10)...
    printf("\nEnter Plane ID: ");
    scanf("%d", & p_id);
    fd.planeID = p_id;

    key_t qKey;

    if((qKey = ftok("airtrafficcontroller.c",'A'))==-1)
  {
    printf("Error in generating key!!!!!");
    exit(0);
  }
  
    

    int Plane_Typ;

    /* 1 for passenger plane
       0 for cargo plane */

    printf("\nEnter Type of Plane: ");
    scanf("%d", & Plane_Typ);
    fd.planeType = Plane_Typ;
    
    int twgt = 0; //total Non Crew(Passenger or Cargo) weight in the plane...

    if (Plane_Typ == 1) {
    //Passenger Plane Process...
      int pwgt;
      int tSeats;
      //Total Occupied Seats (1-10)...
      printf("\nEnter Number of Occupied Seats: ");
      scanf("%d", & tSeats);
      fd.numItems = tSeats;

      

      for (int i = 0; i < tSeats; i++) 
      {
        int arr[2];
        if (pipe(arr) == -1) {
          printf("\nPipe Creation Failed");
          exit(0);
        }
        pid_t pid = fork();
        if (pid == -1) {
          printf("\nFork execution Error");
          exit(0);
        } 
        else if (pid == 0) {
          printf("\nEnter details for Passenger %d =>",i+1);
          pDetails pd;
          close(arr[read_end]);

          int lwgt;
          printf("\nEnter Weight of Your Luggage: ");
          scanf("%d", & lwgt);

          int bwgt;
          printf("\nEnter Your Body Weight: ");
          scanf("%d", & bwgt);

          pd.bWeight = bwgt;
          pd.lWeight = lwgt;

          write(arr[write_end], & pd, sizeof(pDetails)); 
          close(arr[write_end]);

            return 0;
          }
          else {
            wait(NULL);
            pDetails pd;

            close(arr[write_end]);

            read(arr[read_end], & pd, sizeof(pDetails));
            pwgt += (pd.bWeight + pd.lWeight);

            close(arr[read_end]);

          }

        }
        
        twgt = (pwgt) + (avgCrewWgt*7); //7 crew members
      }
      
      else{
          //if Cargo Plane Process...
          int numCargo;
          printf("\nEnter Number of Cargo Items: ");
          scanf("%d",&numCargo);
          fd.numItems = numCargo;
          
          int avgWeight;
          printf("\nEnter Average Weight of Cargo Items: ");
          scanf("%d",&avgWeight);
          
          twgt = (avgWeight*numCargo) + (avgCrewWgt*2);  //2 pilot only
          
      }
      
      int ACode,DCode;
      
      //Both codes should have valid range[1-10];
      printf("\nEnter Airport Number for Departure: "); 
      scanf("%d",&DCode);
      
      printf("\nEnter Airport Number for Arrival: ");
      scanf("%d",&ACode);
      
      fd.Arrival_Code = ACode;
      
      fd.Departure_Code = DCode;
      
      fd.Total_Weight = twgt;
      
      fd.lStats = false;
      
      fd.ulStats = false;
      
      fd.sent = false;
      
      MSG.mtype = 12;
      MSG.Fd = fd;
      
      int msgID;
      msgID = msgget(qKey,IPC_CREAT|0666); //Access the message queue created in airtrafficcontroller.c
      if(msgID == -1)
      {
       printf("\nMessage Queue not Found!!!!!");
       exit(0);
      }
      printf("\nmsgid: %d",msgID);
      
      //sending flight details to ATC...
      if(msgsnd(msgID,(void*)&MSG,sizeof(Flight_Details),0)==-1)
      {
         printf("\nMessage not Sent");
         exit(0);
      }
      
      //receiving termination order from ATC...
      Cleanup t_req;
      
      while(1)
      {
       if(msgrcv(msgID,(void*)&t_req,sizeof(bool),13,IPC_NOWAIT)!=-1)
       {
         if(t_req.initTermination)
         {
         break;
         }
       }
      }
      
      printf("\nPlane %d has successfully traveled from Airport %d to Airport %d!\n",p_id,DCode,ACode);
      return 0;
    }
