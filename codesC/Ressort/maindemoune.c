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

#define BUF_SIZE 4

static int exitRequested = 0;
/*
* sigintHandler --
*
* SIGINT handler, so we can gracefully exit when the user hits ctrl-C.
*/
static void sigintHandler(int signum) { exitRequested = 1; }

int main(int argc, char **argv)
{
  struct ftdi_context *ftdi;
  struct ftdi_version_info version;
  int vid = 0;
  int pid = 0;
  int f = 0, i = 0;
  int baudrate = 115200;//230400;
  int interface = INTERFACE_ANY;
  int retval = EXIT_FAILURE;

  // Debugging infos
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
  if (!vid && !pid && (interface == INTERFACE_ANY))
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
    f = ftdi_usb_open(ftdi, vid, pid);
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
  double posSM_d; // Position in degrees
  double torq; // Torque in mNm
  unsigned int torq_u16;
  const double torqMax = 150; // in mNm
  const double incToDeg = 360/20000.;
  const double slope = -1; // in mNm/deg
  struct timeb prev, now;
  ftime(&prev);
  unsigned int loop=1, n=1000, diff;
  int a,b,c;
  i =0;
  f=0;
   while(f==0)
    f = ftdi_read_data(ftdi, bufread, sizeof(bufread));
    printf("%d",f);
    for (i;i<3;i++)
    {
        if (bufread[i] == 5)
        {
            a=i;
            b=(i+1)%3;
            c=(i+2)%3;
        }

    }
  while (!exitRequested)
  {
    f = ftdi_read_data(ftdi, bufread, sizeof(bufread));

//    for (i=0;i<f;i++) printf("%d ",bufread[i]);
       //printf("f=%d\n",f);

    if (f>0) {
        //    printf("buffers %d %d %d \n",bufread[a],bufread[b],bufread[c]);

      //  if (bufread [0] == 5) {
            posSM_i = bufread[2]*256+bufread[1];
            if (posSM_i > 32767) posSM_i -= 65536;
      //  }
   //   printf("buffers %d %d %d %d\n",bufread[0],bufread[1],bufread[2],posSM_i);
//        f = ftdi_usb_purge_tx_buffer(ftdi);
//        if (f < 0)
//        {
//            fprintf(stderr, "unable to purge RX buffer: %d (%s)\n", f, ftdi_get_error_string(ftdi));
//            exit(-1);
//        }
    }
    posSM_d = posSM_i*incToDeg;
/* ********************************************** RESSORT *************************************************************************** */
    //torq = slope*posSM_d;

    if(posSM_d >= 30)
    {
        torq = -20*(posSM_d-30);
        //torq = 30;
    }
    else
    {
        if(posSM_d <= -30)
        {
               torq = -20*(posSM_d+30);

        }
        else
        {//torq = 20*(posSM_d+10);
        torq = 0;
        if(posSM_d >= 12.5 && posSM_d <= 15.5)
        {
        torq = 2*cos((posSM_d-15)*2*2*3.14);
        }
         if(posSM_d >= -20 && posSM_d <= -10)
        {
            ftime(&now);
            torq = 2*cos((now.millitm)*500*2*3.14);
     //   torq = 2*cos((posSM_d-15)*2*2*3.14);
        }
        }
    }
/* *********************************************FIN RESSORT******************************************************************************* */

/* ********************************************* mur amorti **************************************************************************
    test = test/(max/20) ;
    pente = exp(test)-1;
/* ********************************************FIN MUR AMORTI************************************************************************** */

    torq = min(max(torq,-torqMax),torqMax);
    torq_u16 = 32767*(1+torq/torqMax);

    //torq_u16=115;

    bufwrite[0] = (torq_u16 & (0b00111111) );
    bufwrite[1] = ((torq_u16 >> 6) & 0b00111111) | 0b01000000  ;
    bufwrite[2] = ((torq_u16 >> 12) & 0b00111111) | 0b10000000 ;
    bufwrite[3] = 0b11000000;

    f = ftdi_write_data(ftdi,bufwrite,4);
   // f = ftdi_write_data_submit(ftdi,bufwrite,4);
   // f = ftdi_write_data_submit(ftdi,bufwrite,4);
  //  printf("wrote %d\t",f);
   // printf("Position = %6d inc = %6.2f\xF8, Torque = %6.2f mNm = %2d+256*%2d \n",posSM_i,posSM_d,torq,bufwrite[1],bufwrite[2]);

//printf("%d buffers %d %d %d \n",torq_u16,bufread[0],bufread[1],bufread[2]);

#if defined(_WIN32)
//   Sleep(50);
#endif
    if (loop%n==.0) {
        ftime(&now);
        diff = (int) (1000.0 * (now.time - prev.time)+ (now.millitm - prev.millitm));
        printf("Position = %6d inc = %6.2f\xF8, Torque = %6.2f mNm = %2d+256*%2d \n",posSM_i,posSM_d,torq,bufwrite[1],bufwrite[2]);
		printf("%u loops in %u ms: %4.1f Hz\n",n,diff,1000*n/(float)(diff));
		ftime(&prev);
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
