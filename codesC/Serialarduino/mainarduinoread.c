#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#define n 1024

//Globals
HANDLE hSerial;                                            //Handle for serial port
FILE *fd;                                                //File Descriptor of file to write to serial handle

//Prototypes
int open_port();                                        //Opens serial port for reading and writing
int read();                                                //Reads information from serial port



int main () {

   int read();

   return 0;

}

int read()
{

   DWORD dwBytesRead = 0;
   printf("\nAttempring to read from serial port?");

   char szBuff[n + 1] = {0};
   int intBuff;
   int buffSum
   float floatBuff;

   while(1)
   {

       ReadFile(hSerial, szBuff, 1, &dwBytesRead, NULL); //get a char from the serial line
       if(szBuff[0]=="\n") //if it's "\n" we've hit the beginning of a number start pulling it.
       {

           ReadFile(hSerial, szBuff, 4, &dwBytesRead, NULL); //pull all 4 chars
           for(int i=1,i<=4,i++){

               intBuff = atoi(szBuff[i]); //format the chars into ints
               buffSum += intBuff*10^(i-1); //make a big ole int out of the 4 values
               floatBuff = buffSum/1000 //make it a float

               setValue(char* table, char* depth, float floatBuff); //send the depth to the mySQL DB


           }

       }

       szBuff = {0};
       break;

   }
   return 1;

}


int open_port()
{

   //Tell user that port-opening has been initiated
   printf("Opening Serial Port?\n");

   //Announce current task
   printf("Attaching handle?");

   //Attempt to attach a hande to serial port
   hSerial = CreateFile(L"COM3",

           GENERIC_READ | GENERIC_WRITE,
           0,
           0,
           OPEN_EXISTING,
           FILE_ATTRIBUTE_NORMAL,
           0);

   if(hSerial==INVALID_HANDLE_VALUE)
   {

       if(GetLastError()==ERROR_FILE_NOT_FOUND){

           //Print Error if neccessary
           printf("ERROR: Handle was not attached.  Reason: COM3 not available.\n");
           return 0;

       }
       //Print Error if neccessary
       printf("\n\nWelp?that didn't work.  Explaination: it blew up?pretty much?");
       return 0;

   }
   else printf("done!");                                //Successfully Attached handle to serial port

   //Announce current task
   printf("\nSetting Parameters?");

   //Set serial port parameters

   DCB dcbSerialParams = {0};
   //dcbSerial.DCBlength=sizeof(dcbSerialParams);
   if (!GetCommState(hSerial, &dcbSerialParams)) {

       printf("failed!");
       return 0;

   }
   dcbSerialParams.BaudRate=CBR_9600;
   dcbSerialParams.ByteSize=8;
   dcbSerialParams.StopBits=ONESTOPBIT;
   dcbSerialParams.Parity=NOPARITY;
   if(!SetCommState(hSerial, &dcbSerialParams)){

       printf("\n\nALERT: Serial port failed!");

   }
   else printf("done!");                        //Serial port has been successfully configured
   return 1;

}