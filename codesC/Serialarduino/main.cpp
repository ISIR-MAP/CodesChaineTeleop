/* 	Inspired from https://github.com/legege/libftdi/blob/master/examples/serial_test.c
 *
    Read/write data via serial I/O

    This program is distributed under the GPL, version 2
*/

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <vector>
#include <unistd.h>
#include <string>
#include "SerialClass.h"  // Library described above
#include <conio.h>

#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#define SERVER "127.0.0.1"  //ip address of udp server
#define BUFLEN 24  //Max length of buffer
#define PORT 5009   //The port on which to listen for incoming data

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

BOOL bNewBehavior = FALSE;
DWORD dwBytesReturned = 0;

    using namespace std;
    static int exitRequested = 0;
/*
* sigintHandler --
*
* SIGINT handler, so we can gracefully exit when the user hits ctrl-C.
*/
static void sigintHandler(int signum) { exitRequested = 1; }

int main(int argc, char **argv)
{
  int f = 0, i = 0, m, g;
  unsigned char buffer[MAX_PATH];
  unsigned char digits[MAX_PATH];
  int parse_hex_bytes = 0;
  int baudrate = 9600;//115200;//230400;
 double A;
  A = 120.0;
  double taucible;
  taucible = 0.0;
  char text_to_send[MAX_PATH];
  int dev_num = 50;
  int retval = EXIT_FAILURE;
  char dev_name[MAX_PATH];
  HANDLE hSerial;
  DCB dcbSerialParams = {0};
  int demarage = 1;
  // Debugging infos
  char com[256];
  strcpy(com, "");
  while ((i = getopt(argc, argv, "c:f:b:t:")) != -1)
  {
    switch (i)
    {
      case 'c':
      strcpy(com, optarg);
      break;
      case 'f': // 0=ANY, 1=A, 2=B, 3=C, 4=D
      A = strtoul(optarg, NULL, 0);
      break;
      case 'b':
      baudrate = strtoul(optarg, NULL, 0);
      break;
      case 't':
      taucible = strtod(optarg, NULL);
      break;
      default:
      fprintf(stderr, "usage: %s [-c COM port] [-f force max] [-b baudrate] [-t taucible] \n", *argv);
      exit(-1);
    }
  }
  printf("We're connecting\n");
  if(strlen(com) == 0)
  {
    printf("Port:");
    scanf("%s",com);
  }
Serial* SP = new Serial(com);    // adjust as needed

if (SP->IsConnected())
  printf("We're connected\n");

struct sockaddr_in si_other, server;
    int s, slen=sizeof(si_other), recv_len;
    char buf[BUFLEN];
    char message[BUFLEN];
    WSADATA wsa;

    //Initialise winsock
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    printf("Initialised.\n");

    //create socket
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
    {
        printf("socket() failed with error code : %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    //setup address structure
    memset((char *) &si_other, 0, sizeof(si_other));
    server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( PORT );
    u_long iMode = 0;
    ioctlsocket(s, FIONBIO, &iMode);
if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		//exit(EXIT_FAILURE);
	}
	puts("Bind done");

  char incomingData[256] = "";      // don't forget to pre-allocate memory
  //printf("%s\n",incomingData);
  int dataLength = 255;
  int readResult = 0;
  unsigned char bufwrite[4]={0,0,0,0};
  unsigned char bufread[3]={55,0,0};
  static int posSM_i = 0; // Position in increments
  double posSM_d = 0; // Position in degrees
  double torq=0; // Torque in mNm
  unsigned int torq_u16;
  const double torqMax = 130; // in mNm
  const double incToDeg = 360/20000.;
  char position[BUFLEN];
  const double slope = -1; // in mNm/deg
  //struct timeb prev, now, start;
  LARGE_INTEGER prev, now, start;
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);

  unsigned int loop=1, n=1000, diff;
  int a,b,c;
  i =0;
  f=0;
  FILE *fp;


printf("Debut boucle");
 QueryPerformanceCounter(&prev);
  QueryPerformanceCounter(&start);
while (!exitRequested)
{
  QueryPerformanceCounter(&now);
  diff = (int)(1000.0 * (now.QuadPart - start.QuadPart)/frequency.QuadPart);
  if ((recv_len = recvfrom(s, position, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			//exit(EXIT_FAILURE);
		}

		//torq = atof(position);//304000*4096;
		int test;
		sscanf(position,"%d",&test);
		//torq = test;
		//printf("Result: %d\n",position ,test);
      sprintf(text_to_send,"%d\r\n",test);
      SP->WriteData(text_to_send,strlen(text_to_send));


  //  printf("wrote %d\t",f);
    //printf("Position = %6d inc = %6.2f\xF8, Torque = %6.2f mNm = %2d+256*%2d \n",posSM_i,posSM_d,torq,bufwrite[1],bufwrite[2]);

//printf("%d buffers %d %d %d \n",torq_u16,bufread[0],bufread[1],bufread[2]);

#if defined(_WIN32)
//   Sleep(50);
#endif
  if (loop%n==.0) {
    QueryPerformanceCounter(&now);
    diff = (int)(1000.0 * (now.QuadPart - prev.QuadPart)/frequency.QuadPart);

    printf("%u loops in %u ms: %4.1f Hz %d\n",n,diff,1000*n/(float)(diff), test);

		// printf("%d\n",t);
    QueryPerformanceCounter(&prev);

  }
  loop++;
}
return 0;}



