/* 	Inspired from https://github.com/legege/libftdi/blob/master/examples/serial_test.c
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

#include <conio.h>

/*=============================================================================
  Définition de constantes
=============================================================================*/
#define RX_SIZE         16    /* taille tampon d'entrée  */
#define TX_SIZE         16    /* taille tampon de sortie */
#define MAX_WAIT_READ   1    /* temps max d'attente pour lecture (en ms) */


/*=============================================================================
  Variables globales.
=============================================================================*/

/* Handle du port COM1 ouvert */
HANDLE g_hCOM = NULL;

/* Handle du port COM2 ouvert */
HANDLE g_hCOM2 = NULL;

/* Délais d'attente sur le port COM */
COMMTIMEOUTS g_cto =
{
    MAX_WAIT_READ,  /* ReadIntervalTimeOut          */
    0,              /* ReadTotalTimeOutMultiplier   */
    MAX_WAIT_READ,  /* ReadTotalTimeOutConstant     */
    0,              /* WriteTotalTimeOutMultiplier  */
    0               /* WriteTotalTimeOutConstant    */
};


COMMTIMEOUTS g_cto2 =
{
    MAX_WAIT_READ,  /* ReadIntervalTimeOut          */
    0,              /* ReadTotalTimeOutMultiplier   */
    MAX_WAIT_READ,  /* ReadTotalTimeOutConstant     */
    0,              /* WriteTotalTimeOutMultiplier  */
    0               /* WriteTotalTimeOutConstant    */
};

/* Configuration du port COM1 */
DCB g_dcb =
{
    sizeof(DCB),        /* DCBlength            */
    9600,               /* BaudRate             */
    TRUE,               /* fBinary              */
    FALSE,              /* fParity              */
    FALSE,              /* fOutxCtsFlow         */
    FALSE,              /* fOutxDsrFlow         */
    DTR_CONTROL_ENABLE, /* fDtrControl          */
    FALSE,              /* fDsrSensitivity      */
    FALSE,              /* fTXContinueOnXoff    */
    FALSE,              /* fOutX                */
    FALSE,              /* fInX                 */
    FALSE,              /* fErrorChar           */
    FALSE,              /* fNull                */
    RTS_CONTROL_ENABLE, /* fRtsControl          */
    FALSE,              /* fAbortOnError        */
    0,                  /* fDummy2              */
    0,                  /* wReserved            */
    0x100,              /* XonLim               */
    0x100,              /* XoffLim              */
    8,                  /* ByteSize             */
    EVENPARITY,           /* Parity               */
    ONESTOPBIT,         /* StopBits             */
    0x11,               /* XonChar              */
    0x13,               /* XoffChar             */
    '?',                /* ErrorChar            */
    0x1A,               /* EofChar              */
    0x10                /* EvtChar              */
};



/* Configuration du port COM2 */
DCB g_dcb2 =
{
    sizeof(DCB),        /* DCBlength            */
    19200,               /* BaudRate             */
    TRUE,               /* fBinary              */
    FALSE,              /* fParity              */
    FALSE,              /* fOutxCtsFlow         */
    FALSE,              /* fOutxDsrFlow         */
    DTR_CONTROL_ENABLE, /* fDtrControl          */
    FALSE,              /* fDsrSensitivity      */
    FALSE,              /* fTXContinueOnXoff    */
    FALSE,              /* fOutX                */
    FALSE,              /* fInX                 */
    FALSE,              /* fErrorChar           */
    FALSE,              /* fNull                */
    RTS_CONTROL_ENABLE, /* fRtsControl          */
    FALSE,              /* fAbortOnError        */
    0,                  /* fDummy2              */
    0,                  /* wReserved            */
    0x100,              /* XonLim               */
    0x100,              /* XoffLim              */
    8,                  /* ByteSize             */
    EVENPARITY,           /* Parity               */
    ONESTOPBIT,         /* StopBits             */
    0x11,               /* XonChar              */
    0x13,               /* XoffChar             */
    '?',                /* ErrorChar            */
    0x1A,               /* EofChar              */
    0x10                /* EvtChar              */
};

/*=============================================================================
  Fonctions du module.
=============================================================================*/
BOOL OpenCOM    (int nId);
BOOL CloseCOM   ();
BOOL ReadCOM    (void* buffer, int nBytesToRead, int* pBytesRead);
BOOL WriteCOM   (void* buffer, int nBytesToWrite, int* pBytesWritten);


BOOL OpenCOM2    (int nId2);
BOOL CloseCOM2   ();
BOOL ReadCOM2    (void* buffer2, int nBytesToRead2, int* pBytesRead2);
BOOL WriteCOM2   (void* buffer2, int nBytesToWrite2, int* pBytesWritten2);

//#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDORh, 12)

BOOL bNewBehavior = FALSE;
DWORD dwBytesReturned = 0;

//#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
static int exitRequested = 0;
/* sigintHandler -- SIGINT handler, so we can
gracefully exit when the user hits ctrl-C. */
static void sigintHandler(int signum)
{
    exitRequested = 1;
}

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

    unsigned char bufwrite[4]= {0,0,0,0};
    unsigned char bufread[3]= {55,0,0,0};
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

    /* variables locales */
    char buffer[10];
    int nId, nChoice, nBytesWritten, nBytesRead;

    char buffer2[10];
    int nId2, nChoice2, nBytesWritten2, nBytesRead2;


    int lecture, lect;

    /* demande du numéro du port COM */
    printf("Entrez le numero du port COM1 : ");
    scanf("%d", &nId);


    /* demande du numéro du port COM2 */
    printf("Entrez le numero du port COM2 : ");
    scanf("%d", &nId2);



    /* tentative d'ouverture */
    printf("Ouverture et configuration du port COM%d...\r\n", nId);
    if(!OpenCOM(nId)) return -1;
    printf("...OK\r\n");


    /* tentative d'ouverture 2 */
    printf("Ouverture et configuration du port COM%d...\r\n", nId2);
    if(!OpenCOM2(nId2)) return -1;
    printf("...OK\r\n");

    while (!exitRequested)
    {
        lecture = read_arduino(g_hCOM2, lecture);
        torq = (lecture)/2048.0*130.0;
        torq = min(max(torq,-torqMax),torqMax);

        bufwrite[0] = (torq_u16 & (0b00111111) );
        bufwrite[1] = ((torq_u16 >> 6) & 0b00111111) | 0b01000000  ;
        bufwrite[2] = ((torq_u16 >> 12) & 0b00111111) | 0b10000000 ;
        bufwrite[3] = 0b11000000;
        tc = ftdi_write_data_submit(ftdi,bufwrite,4);
        f = ftdi_transfer_data_done(tc);
        torq_u16 = 32767*(1+torq/torqMax);
        f = ftdi_read_data(ftdi, bufread, sizeof(bufread));
        if (bufread[0] != 5)
        {
            while(bufread[0] != 5)
            {
                f = ftdi_read_data(ftdi, bufread, sizeof(bufread)/3);
            }
            f = ftdi_read_data(ftdi, bufread, sizeof(bufread)*2/3);
            f = ftdi_read_data(ftdi, bufread, sizeof(bufread));

        }
        if (bufread[0] == 5)
        {
            if (f>0)
            {
                posSM_i = bufread[2]*256+bufread[1];
                if (posSM_i > 32767) posSM_i -= 65536;
                if (posSM_i > 4096) posSM_i = 4096;
                if (posSM_i < 0) posSM_i = 0;
                sprintf(buffer,"%i\r\n",posSM_i);
               if (loop%n==.0)
        { WriteCOM(buffer, strlen(buffer), &nBytesWritten);}

            }
        }

#if defined(_WIN32)
//   Sleep(50);
#endif
        if (loop%n==.0)
        {
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
/******************************************************************************
  OpenCOM : ouverture et configuration du port COM.
  entrée : nId : Id du port COM à ouvrir.
  retour : vrai si l'opération a réussi, faux sinon.
******************************************************************************/
BOOL OpenCOM(int nId)
{
    /* variables locales */
    char szCOM[16];

    /* construction du nom du port, tentative d'ouverture */
    sprintf(szCOM, "COM%d", nId);
    g_hCOM = CreateFile(szCOM, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);
    if(g_hCOM == INVALID_HANDLE_VALUE)
    {
        printf("Erreur lors de l'ouverture du port COM%d", nId);
        return FALSE;
    }

    /* affectation taille des tampons d'émission et de réception */
    SetupComm(g_hCOM, RX_SIZE, TX_SIZE);

    /* configuration du port COM */
    if(!SetCommTimeouts(g_hCOM, &g_cto) || !SetCommState(g_hCOM, &g_dcb))
    {
        printf("Erreur lors de la configuration du port COM%d", nId);
        CloseHandle(g_hCOM);
        return FALSE;
    }

    /* on vide les tampons d'émission et de réception, mise à 1 DTR */
    PurgeComm(g_hCOM, PURGE_TXCLEAR|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_RXABORT);
    EscapeCommFunction(g_hCOM, SETDTR);
    return TRUE;
}

BOOL OpenCOM2(int nId2)
{
    /* variables locales */
    char szCOM2[16];
    /* construction du nom du port, tentative d'ouverture */
    sprintf(szCOM2, "COM%d", nId2);
    g_hCOM2 = CreateFile(szCOM2, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);
    if(g_hCOM2 == INVALID_HANDLE_VALUE)
    {
        printf("Erreur lors de l'ouverture du port COM2%d", nId2);
        return FALSE;
    }
    /* affectation taille des tampons d'émission et de réception */
    SetupComm(g_hCOM2, RX_SIZE, TX_SIZE);
    /* configuration du port COM */
    if(!SetCommTimeouts(g_hCOM2, &g_cto2) || !SetCommState(g_hCOM2, &g_dcb2))
    {
        printf("Erreur lors de la configuration du port COM2%d", nId2);
        CloseHandle(g_hCOM2);
        return FALSE;
    }
    /* on vide les tampons d'émission et de réception, mise à 1 DTR */
    PurgeComm(g_hCOM2, PURGE_TXCLEAR|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_RXABORT);
    EscapeCommFunction(g_hCOM2, SETDTR);
    return TRUE;
}

/******************************************************************************
  CloseCOM : fermeture du port COM.
  retour : vrai si l'opération a réussi, faux sinon.
******************************************************************************/
BOOL CloseCOM()
{
    /* fermeture du port COM */
    CloseHandle(g_hCOM);
    return TRUE;
}

BOOL CloseCOM2()
{
    /* fermeture du port COM */
    CloseHandle(g_hCOM2);
    return TRUE;
}
/******************************************************************************
  ReadCOM : lecture de données sur le port COM.
  entrée : buffer       : buffer où mettre les données lues.
           nBytesToRead : nombre max d'octets à lire.
           pBytesRead   : variable qui va recevoir le nombre d'octets lus.
  retour : vrai si l'opération a réussi, faux sinon.
-------------------------------------------------------------------------------
  Remarques : - la constante MAX_WAIT_READ utilisée dans la structure
                COMMTIMEOUTS permet de limiter le temps d'attente si aucun
                caractères n'est présent dans le tampon d'entrée.
              - la fonction peut donc retourner vrai sans avoir lu de données.
******************************************************************************/

BOOL ReadCOM(void* buffer, int nBytesToRead, int* pBytesRead)
{
    return ReadFile(g_hCOM, buffer, nBytesToRead, pBytesRead, NULL);
}

BOOL ReadCOM2(void* buffer2, int nBytesToRead2, int* pBytesRead2)
{
    return ReadFile(g_hCOM2, buffer2, nBytesToRead2, pBytesRead2, NULL);
}

/******************************************************************************
  WriteCOM : envoi de données sur le port COM.
  entrée : buffer        : buffer avec les données à envoyer.
           nBytesToWrite : nombre d'octets à envoyer.
           pBytesWritten : variable qui va recevoir le nombre d'octets
                           envoyés.
  retour : vrai si l'opération a réussi, faux sinon.
******************************************************************************/
BOOL WriteCOM(void* buffer, int nBytesToWrite, int* pBytesWritten)
{
    /* écriture sur le port */
    return WriteFile(g_hCOM, buffer, nBytesToWrite, pBytesWritten, NULL);
}

BOOL WriteCOM2(void* buffer2, int nBytesToWrite2, int* pBytesWritten2)
{
    /* écriture sur le port */
    return WriteFile(g_hCOM2, buffer2, nBytesToWrite2, pBytesWritten2, NULL);
}

int read_arduino (HANDLE hserial, int shill)
{
    char buffer[10];       // any value longer than 3 digits must come
                          // from a faulty transmission
                          // the 4th caracter is used for a terminating '\0'
    size_t buf_index = 0; // storage position of received characters
    for (;;)
    {
        char c; // read one byte at a time

        if (!ReadFile(
            hserial,
            &c,   // 1 byte buffer
            1,    // of length 1
            NULL, // we will read exactly one byte or die trying,
                  // so length checking is pointless
            NULL)){
            /*
             * This error means something is wrong with serial port config,
             * and I assume your port configuration is hard-coded,
             * so the code won't work unless you modify and recompile it.
             * No point in keeping the progam running, then.
             */
            fprintf (stderr, "Dang! Messed up the serial port config AGAIN!");
            exit(-1);
        }
        else // our read succeded. That's a start.
        {
            if (c == '\n') // we're done receiving a complete value
            {
                int result; // the decoded value we might return

                // check for buffer overflow
                if (buf_index == sizeof (buffer))
                {
                    // warn the user and discard the input
                   // fprintf (stderr,
                       // "Too many characters received, input flushed\n");
                       //printf("Erreur");
                     //  printf("%d \r\n", shill);
                }
                else // valid number of characters received
                {
                    // add a string terminator to the buffer
                    buffer[buf_index] = '\0';

                    // convert to integer
                    result = atoi (buffer);


                        return result; // <-- this is the only exit point
                }

                // reset buffer index to prepare receiving the next line
                buf_index = 0;
            }
            else // character other than '\n' received
            {
                // store it as long as our buffer does not overflow
                if (buf_index < sizeof (buffer))
                {
                    buffer[buf_index++] = c;
/*
 * if, for some reason, we receive more than the expected max number of
 * characters, the input will be discarded until the next '\n' allow us
 * to re-synchronize.
 */
                }
            }
        }
    }
}




int read_arduino1 (HANDLE hserial)
{
    char buffer[10];       // any value longer than 3 digits must come
                          // from a faulty transmission
                          // the 4th caracter is used for a terminating '\0'
    size_t buf_index = 0; // storage position of received characters
    for (;;)
    {
        char c; // read one byte at a time

        if (!ReadFile(
            hserial,
            &c,   // 1 byte buffer
            1,    // of length 1
            NULL, // we will read exactly one byte or die trying,
                  // so length checking is pointless
            NULL)){
            /*
             * This error means something is wrong with serial port config,
             * and I assume your port configuration is hard-coded,
             * so the code won't work unless you modify and recompile it.
             * No point in keeping the progam running, then.
             */
            fprintf (stderr, "Dang! Messed up the serial port config AGAIN!");
            exit(-1);
        }
        else // our read succeded. That's a start.
        {
            if (c == '\n') // we're done receiving a complete value
            {
                int result; // the decoded value we might return

                // check for buffer overflow
                if (buf_index == sizeof (buffer))
                {
                    // warn the user and discard the input
                    fprintf (stderr,
                        "Too many characters received, input flushed\n");
                       //printf("Erreur");
                       //printf("%d \r\n", shill);
                }
                else // valid number of characters received
                {
                    // add a string terminator to the buffer
                    buffer[buf_index] = '\0';

                    // convert to integer
                    result = atoi (buffer);


                        return result; // <-- this is the only exit point
                }

                // reset buffer index to prepare receiving the next line
                buf_index = 0;
            }
            else // character other than '\n' received
            {
                // store it as long as our buffer does not overflow
                if (buf_index < sizeof (buffer))
                {
                    buffer[buf_index++] = c;
/*
 * if, for some reason, we receive more than the expected max number of
 * characters, the input will be discarded until the next '\n' allow us
 * to re-synchronize.
 */
                }
            }
        }
    }
}
