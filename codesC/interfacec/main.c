
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
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

/******************************************************************************
  main : point d'entrée du programme.
******************************************************************************/
int main()
{
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

    /* boucle tant que l'on ne quitte pas */
    do
    {
        /* menu */
        printf("\r\n");
        printf("1 : Envoyer des donnees a COM1.\r\n");
        printf("2 : Recevoir des donnees de COM1.\r\n");
        printf("3 : Envoyer des donnees a COM2.\r\n");
        printf("4 : Recevoir des donnees de COM2.\r\n");
        printf("5 : Quitter.\r\n");
        printf("Choix : ");
        scanf("%d", &nChoice);

        /* enoyer des données */
        if(nChoice == 1)
        {
            printf("\r\n");
            printf("Donnees a envoyer :\r\n");
            fflush(stdin);
            //gets(buffer);
            int test;
            scanf("%d",&test);
            sprintf(buffer,"%d\r\n",test);
            printf("\r\n");
            printf("Envoi des donnees...\r\n");
            if(WriteCOM(buffer, strlen(buffer), &nBytesWritten))
                printf("%d octet(s) envoye(s).\r\n", nBytesWritten);
            else
                printf("Erreur lors de l'envoi.\r\n");
        }

        /* recevoir des données */
        if(nChoice == 2)
        {
            printf("\r\n");
            printf("Reception de donnees...\r\n");
            int grg1 = 0;
           // int lecture;

             while(1)
            {

            //grg += 1;
            //while(grg > 10) {
           // fflush(stdout);
            lecture = read_arduino(g_hCOM2, lecture);
            //printf("tt\n");

           // fflush(stdout);
            //printf("%d \r\n", lecture);
            //grg = 0;
            //}
            //printf("%d \r\n", lecture);
           }

            /*
            while(1) {

            printf("alo...\r\n");
            lect = read_arduino1(g_hCOM);
            printf("%d \r\n", lect);

            }
            */

        //  if(ReadCOM(buffer, sizeof(buffer), &nBytesRead))
          //  {
            //    buffer[nBytesRead] = '\0';
              //  printf("%d octet(s) recu(s) :\r\n%s\r\n", nBytesRead, buffer);
            //}
            /*else
                printf("Erreur lors de la réception.\r\n");*/
        }

if(nChoice == 3)
        {
            printf("\r\n");
            printf("Donnees a envoyer :\r\n");
            fflush(stdin);
            //gets(buffer);
            int test2;
            scanf("%d",&test2);
            sprintf(buffer2,"%d\r\n",test2);
            printf("\r\n");
            printf("Envoi des donnees...\r\n");
            if(WriteCOM2(buffer2, strlen(buffer2), &nBytesWritten2))
                printf("%d octet(s) envoye(s).\r\n", nBytesWritten2);
            else
                printf("Erreur lors de l'envoi.\r\n");
        }

        /* recevoir des données */    // COM2
        if(nChoice == 4)
        {
            printf("\r\n");
            printf("Reception de donnees...\r\n");
int modulo = 0;
             while(1)
            {

           lecture = read_arduino(g_hCOM2, lecture);
           modulo++;
           if(modulo >= 10)
           {
               printf("%d \r\n", lecture);
               sprintf(buffer,"%d\r\n",lecture+2048);
            WriteCOM(buffer, strlen(buffer), &nBytesWritten);
    modulo=0;
           }
           }
         /*  if(ReadCOM2(buffer2, sizeof(buffer2), &nBytesRead2))
            {
                buffer2[nBytesRead2] = '\0';
                printf("%d octet(s) recu(s) :\r\n%s\r\n", nBytesRead2, buffer2);
            }
            else
                printf("Erreur lors de la réception.\r\n");
        */
        }
    }while(nChoice != 5);
    /* fermeture du port COM et retour */
    CloseCOM();
    CloseCOM2();
    return 0;
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
