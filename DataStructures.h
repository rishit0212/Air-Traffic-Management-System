/*
  !!!!Save this file in same folder as Others!!!!
*/

//DataStructure for flight details...
typedef struct {

  int Arrival_Code;
  int Departure_Code;
  int planeID;
  int Total_Weight;
  int planeType;
  int numItems;
  bool lStats;
  bool ulStats;
  bool sent;
 
}Flight_Details; 

//Message structure for cleanup request...
typedef struct {
 long mtype;
 bool initTermination;
}Cleanup;

/*message structure for Flight Details*/
typedef struct {
   long mtype;
   Flight_Details Fd;
   
}Message;



/*Data structure used in Thread in airport.c file*/
typedef struct {
  Message msg;
  int size;
  int *capacity;
  int backCap;
}t_data;


