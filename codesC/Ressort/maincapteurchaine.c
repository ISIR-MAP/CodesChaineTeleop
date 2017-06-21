/* 	Inspired from https://github.com/legege/libftdi/blob/master/examples/serial_test.c
 *
    Read/write data via serial I/O

    This program is distributed under the GPL, version 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <ftdi.h>
#include <libusb.h>
#include <sys\timeb.h>
#include <sys\stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library


#define SERVER "127.0.0.1"  //ip address of udp server
#define BUFLEN 24  //Max length of buffer
#define PORT 5005   //The port on which to listen for incoming data
#define sPORT 5009

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

BOOL bNewBehavior = FALSE;
DWORD dwBytesReturned = 0;

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

static int exitRequested = 0;
/*
* sigintHandler --
*
* SIGINT handler, so we can gracefully exit when the user hits ctrl-C.
*/
static void sigintHandler(int signum) { exitRequested = 1; }

void usleep(__int64 usec)
{
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}

int main(int argc, char **argv)
{
  struct ftdi_context *ftdi;
  struct ftdi_version_info version;
  int vid = 0x0403;
  int pid = 0x6014;
  int f = 0, i = 0;
  int baudrate = 230400;//115200;//230400;
  int interface = INTERFACE_ANY;
  int retval = EXIT_FAILURE;
  int demarage = 1;
  // Debugging infos
const int buf_max = 256;
  int fd = -1;
    char serialport[buf_max];
    int baudrates = 9600;  // default
    char quiet=0;
    char eolchar = '\r\n';
    int timeout = 5000;
    char bufs[buf_max];
int rc;



  version = ftdi_get_library_version();
  printf("Initialized libftdi %s (major: %d, minor: %d, micro: %d, snapshot ver: %s)\n",
         version.version_str, version.major, version.minor, version.micro,
         version.snapshot_str);

  while ((i = getopt(argc, argv, "i:v:p:b:")) != -1)
  {
    switch (i)
    {
      case 'i': // 0=ANY, 1=A, 2=B, 3=C, 4=D
        interface = strtoul(optarg, NULL, 0);
        break;
      case 'v':
        vid = strtoul(optarg, NULL, 0);
        break;
      case 'p':
        pid = strtoul(optarg, NULL, 0);
        break;
      case 'b':
        baudrate = strtoul(optarg, NULL, 0);
        break;
      default:
        fprintf(stderr, "usage: %s [-i interface] [-v vid] [-p pid] [-b baudrate] [-w [pattern]]\n", *argv);
        exit(-1);
    }
  }

  // Initialization
  if ((ftdi = ftdi_new()) == 0)
  {
    fprintf(stderr, "ftdi_new failed\n");
    return EXIT_FAILURE;
  }

  // Open device
  if ( (interface == INTERFACE_ANY))
  {
    struct ftdi_device_list *devlist, *curdev;
    char manufacturer[128], description[128];
    int res;
    if ((res = ftdi_usb_find_all(ftdi, &devlist, 0, 0)) < 0)
    {
      fprintf(stderr, "No FTDI with default VID/PID found\n");
      goto do_deinit;
    }
    if (res == 1)
    {
      f = ftdi_usb_open_dev(ftdi, devlist[0].dev);
    }
    if (res > 1)
    {
      fprintf(stderr, "%d Devices found, please select Device with VID/PID\n", res);
      i = 0;
      for (curdev = devlist; curdev != NULL; i++)
      {
        printf("Device: %d, ", i);
        if ((f = ftdi_usb_get_strings(ftdi, curdev->dev, manufacturer, 128, description, 128, NULL, 0)) < 0)
        {
          fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", f, ftdi_get_error_string(ftdi));
          retval = EXIT_FAILURE;
          ftdi_list_free(&devlist);
          goto do_deinit;
        }
        printf("Manufacturer: %s, Description: %s\n\n", manufacturer, description);
        curdev = curdev->next;
      }
      ftdi_list_free(&devlist);
      goto do_deinit;
    }
    if (res == 0)
    {
      fprintf(stderr, "No Devices found with default VID/PID\n");
      goto do_deinit;
    }
  }
  else
  {
    f = ftdi_usb_open_desc_index(ftdi, vid, pid,NULL,NULL,interface);
  }

  if (f < 0)
  {
    fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(ftdi));
    exit(-1);
  }

  // Set baud rate
  f = ftdi_set_baudrate(ftdi, baudrate);
  if (f < 0)
  {
    fprintf(stderr, "unable to set baudrate: %d (%s)\n", f, ftdi_get_error_string(ftdi));
    exit(-1);
  }

  /* Set line parameters
   *
   * TODO: Make these parameters settable from the command line
   *
   * Parameters are chosen that sending a continuous stream of 0x55
   * should give a square wave
   *
   */
  f = ftdi_set_line_property(ftdi, 8, STOP_BIT_1, NONE);
  if (f < 0)
  {
    fprintf(stderr, "unable to set line parameters: %d (%s)\n", f, ftdi_get_error_string(ftdi));
    exit(-1);
  }

  f = ftdi_set_latency_timer(ftdi,1);
  if (f < 0)
  {
    fprintf(stderr, "unable to set latency parameters: %d (%s)\n", f, ftdi_get_error_string(ftdi));
    exit(-1);
  }

  signal(SIGINT, sigintHandler);

  unsigned char bufwrite[4]={0,0,0,0};
  unsigned char bufread[3]={55,0,0,0};
  static int posSM_i = 0; // Position in increments
  double posSM_d = 0; // Position in degrees
  double torq=0; // Torque in mNm
  unsigned int torq_u16;
  const double torqMax = 150; // in mNm
  const double incToDeg = 360/20000.;
  const double slope = -1; // in mNm/deg
  struct timeb prev, now, start;
  ftime(&prev);
  ftime(&start);
  unsigned int loop=1, n=100, diff;
  struct ftdi_transfer_control* tc = NULL;
  int a,b,c;
  i =0;
  f=0;

    // RESET FOR INITIALIZATION BUFWRITE [255,255,255,255]
   /* bufwrite[0]=255; bufwrite[1]=255; bufwrite[2]=255; bufwrite[3]=255;
    f = ftdi_write_data_submit(ftdi,bufwrite,4);
    printf("buffers %d   %d    %d    %d   \n",bufwrite[0],bufwrite[1],bufwrite[2],bufwrite[3]);

    bufwrite[0]=0; bufwrite[1]=0; bufwrite[2]=0; bufwrite[3]=0;*/
 //   printf("buffers %d   %d    %d    %d   \n",bufwrite[0],bufwrite[1],bufwrite[2],bufwrite[3]);


    //f = ftdi_write_data_submit(ftdi,bufwrite,4);
 //  while(f==0)
 //   f = ftdi_read_data(ftdi, bufread, sizeof(bufread));
   // printf("%d",f);
  /*  for (i;i<3;i++)
    {
        if (bufread[i] == 5)
        {
            a=i;
            b=(i+1)%3;
            c=(i+2)%3;
        }

    }*/
    //printf("buffers %d   %d    %d    %d   \n",bufread[0],bufread[1],bufread[2],bufread[3]);
   // while(1);
   FILE *fp;

/*********************************************/
    // udp communication
    /*********************************************/

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
    si_other.sin_family = AF_INET;
	si_other.sin_port = htons(sPORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);
    u_long iMode = 1;
    ioctlsocket(s, FIONBIO, &iMode);
  fp = fopen("data.txt", "w");
if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }
  while (!exitRequested)
  {
        //clear the buffer by filling null, it might have previously received data
        //memset(buf,'\0', BUFLEN);

        //try to receive some data, this is a blocking call
        do
        {
        recv_len = recv(s, buf, BUFLEN-1, 0);
        if(recv_len > 0){
            // Null byte :)
            // Remove that garbage >:(
            buf[recv_len] = '\0';
          //  printf("Recvbuf: %s\n\n\niResult: %d\n",buf,recv_len);
            continue; // working properly
        }
        else if(recv_len == 0)
            // Connection closed properly
            break;
        else
        {
         //   printf("ERROR! %ld",WSAGetLastError());
            break;
        }
    } while(recv_len > 0);
        torq = (atof(buf)-5000.0)/5000.0*500;
        torq = min(max(torq,-torqMax),torqMax);

    //torq_u16=40;

    bufwrite[0] = (torq_u16 & (0b00111111) );
    bufwrite[1] = ((torq_u16 >> 6) & 0b00111111) | 0b01000000  ;
    bufwrite[2] = ((torq_u16 >> 12) & 0b00111111) | 0b10000000 ;
    bufwrite[3] = 0b11000000;
    tc = ftdi_write_data_submit(ftdi,bufwrite,4);
    f = ftdi_transfer_data_done(tc);
    torq_u16 = 32767*(1+torq/torqMax);

f = ftdi_read_data(ftdi, bufread, sizeof(bufread));
      if (bufread[0] != 5) {
while(bufread[0] != 5)
{
    f = ftdi_read_data(ftdi, bufread, sizeof(bufread)/3);
}
f = ftdi_read_data(ftdi, bufread, sizeof(bufread)*2/3);
f = ftdi_read_data(ftdi, bufread, sizeof(bufread));

      }
      if (bufread[0] == 5)
        {
    if (f>0) {
            posSM_i = bufread[2]*256+bufread[1];
            if (posSM_i > 32767) posSM_i -= 65536;
               sprintf(message,"%i\r\n", posSM_i);
             if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
        {
            printf("sendto() failed with error code : %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
    }
     }




#if defined(_WIN32)
//   Sleep(50);
#endif
    if (loop%n==.0) {
        ftime(&now);
        diff = (int) (1000.0 * (now.time - prev.time)+ (now.millitm - prev.millitm));
        printf("Position = %6d conv = %6.2f\xF8, Torque = %6.2f mNm = %2d+256*%2d \n",posSM_i,posSM_d,torq,bufwrite[1],bufwrite[2]);
        printf("%u loops in %u ms: %4.1f Hz\n",n,diff,1000*n/(float)(diff));
        // printf("%d\n",t);
        ftime(&prev);
        //fclose(fp);
        //fp = fopen("data.txt", "a");

    }
    loop++;

  }
  signal(SIGINT, SIG_DFL);
  retval =  EXIT_SUCCESS;

  ftdi_usb_close(ftdi);
  do_deinit:
  ftdi_free(ftdi);
  printf("ftdi freed\n");
  return retval;

}
