/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************
Diese Programm basiert auf dem Quellcode von Hilscher cifxlinuxsample.c

Modifiziert von Leo Enns Juni.2018

**************************************************************************************/

#include "cifxlinux.h"
#include "cifXEndianess.h"

#include "rcX_Public.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>

#define CIFX_DEV "cifX0"

#ifndef UNREFERENCED_PARAMETER
  #define UNREFERENCED_PARAMETER(a) (a=a)
#endif
/*-----------------------------Edit by Leo :Start-----------------------------*/
union Float2Hex 
{ 
	float fValue; 
	char Hex[4]; 
}; 
/*-----------------------------Edit by Leo :END-----------------------------*/
/*****************************************************************************/
/*! Displays cifX error
*   \param lError     Error code                                             */
/*****************************************************************************/
void ShowError( int32_t lError )
{
  if( lError != CIFX_NO_ERROR)
  {
    char szError[1024] ={0};
    xDriverGetErrorDescription( lError,  szError, sizeof(szError));
    printf("Error: 0x%X, <%s>\n", (unsigned int)lError, szError);
  }
}


/*****************************************************************************/
/*! Displays a hex dump on the debug console (16 bytes per line)
*   \param pbData     Pointer to dump data
*   \param ulDataLen  Length of data dump                                    */
/*****************************************************************************/
void DumpData(unsigned char* pbData, unsigned long ulDataLen)
{
	unsigned long ulIdx;
#ifdef DEBUG
	printf("%s() called\n", __FUNCTION__);
#endif
  for(ulIdx = 0; ulIdx < ulDataLen; ++ulIdx)
  {
    if(0 == (ulIdx % 16))
      printf("\r\n");

    printf("%02X ", pbData[ulIdx]);
  }
  printf("\r\n");
}


/*****************************************************************************/
/*! Dumps a rcX packet to debug console
*   \param ptPacket Pointer to packed being dumped                           */
/*****************************************************************************/
void DumpPacket(CIFX_PACKET* ptPacket)
{
#ifdef DEBUG
	printf("%s() called\n", __FUNCTION__);
#endif
  printf("Dest   : 0x%08lX      ID   : 0x%08lX\r\n",(long unsigned int)HOST_TO_LE32(ptPacket->tHeader.ulDest),  (long unsigned int)HOST_TO_LE32(ptPacket->tHeader.ulId));
  printf("Src    : 0x%08lX      Sta  : 0x%08lX\r\n",(long unsigned int)HOST_TO_LE32(ptPacket->tHeader.ulSrc),   (long unsigned int)HOST_TO_LE32(ptPacket->tHeader.ulState));
  printf("DestID : 0x%08lX      Cmd  : 0x%08lX\r\n",(long unsigned int)HOST_TO_LE32(ptPacket->tHeader.ulDestId),(long unsigned int)HOST_TO_LE32(ptPacket->tHeader.ulCmd));
  printf("SrcID  : 0x%08lX      Ext  : 0x%08lX\r\n",(long unsigned int)HOST_TO_LE32(ptPacket->tHeader.ulSrcId), (long unsigned int)HOST_TO_LE32(ptPacket->tHeader.ulExt));
  printf("Len    : 0x%08lX      Rout : 0x%08lX\r\n",(long unsigned int)HOST_TO_LE32(ptPacket->tHeader.ulLen),   (long unsigned int)HOST_TO_LE32(ptPacket->tHeader.ulRout));

  printf("Data:");
  DumpData(ptPacket->abData, HOST_TO_LE32(ptPacket->tHeader.ulLen));
}


/*****************************************************************************/
/*! Function to display driver information
 *   \param  hDriver  Handle to cifX driver
 *   \param  ptVTable Pointer to cifX API function table
 *   \return CIFX_NO_ERROR on success                                        */
/*****************************************************************************/
void DisplayDriverInformation (void)
{
  int32_t            lRet             = CIFX_NO_ERROR;
  DRIVER_INFORMATION tDriverInfo      = {{0}};
  char               szDrvVersion[32] = "";
  CIFXHANDLE         hDriver          = NULL;
 
  if (CIFX_NO_ERROR == (lRet = xDriverOpen(&hDriver)))
  {
    printf("\n---------- Display Driver Version ----------\n");
    if( CIFX_NO_ERROR != (lRet = xDriverGetInformation(NULL, sizeof(tDriverInfo), &tDriverInfo)) )
      ShowError( lRet);
    else if ( CIFX_NO_ERROR != (lRet = cifXGetDriverVersion( sizeof(szDrvVersion)/sizeof(*szDrvVersion), szDrvVersion)))
      ShowError( lRet);
    else
      printf("Driver Version: %s, based on %.32s \n\n", szDrvVersion, tDriverInfo.abDriverVersion);
    
    /* close previously opened driver */
    xDriverClose(hDriver);
    
  } 
  
  printf(" State = 0x%08X\r\n", (unsigned int)lRet);
  printf("----------------------------------------------------\r\n");
}

/**************************Edit by Leo****************************************/
/*! Function to demonstrate communication channel functionality
*   Packet Transfer and I/O Data exchange
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t ChannelTransf()
{
#ifdef DEBUG
	printf("%s() called\n", __FUNCTION__);
#endif
  CIFXHANDLE hDriver = NULL;
  int32_t    lRet    = xDriverOpen(&hDriver);

  printf("---------- Communication Channel Transfer ----------\r\n");

  if(CIFX_NO_ERROR == lRet)
  {
    /* Driver/Toolkit successfully opened */
    CIFXHANDLE hChannel = NULL;
    lRet = xChannelOpen(NULL, CIFX_DEV, 0, &hChannel);

    if(CIFX_NO_ERROR != lRet)
    {
		printf("Error opening Channel!");
    }else
	{
		CHANNEL_INFORMATION tChannelInfo = {{0}};
		/* Channel successfully opened, so query basic information */
		if( CIFX_NO_ERROR != (lRet = xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo)))
		{
			printf("Error querying system information block\r\n");
		} else
		{
			printf("Communication Channel Info:\r\n");
			printf("Device Number    : %lu\r\n",(long unsigned int)tChannelInfo.ulDeviceNumber);
			printf("Serial Number    : %lu\r\n",(long unsigned int)tChannelInfo.ulSerialNumber);
			printf("Firmware         : %s\r\n", tChannelInfo.abFWName);
			printf("FW Version       : %u.%u.%u build %u\r\n", 
				tChannelInfo.usFWMajor,
				tChannelInfo.usFWMinor,
				tChannelInfo.usFWRevision,
				tChannelInfo.usFWBuild);
			printf("FW Date          : %02u/%02u/%04u\r\n", 
				tChannelInfo.bFWMonth,
				tChannelInfo.bFWDay,
				tChannelInfo.usFWYear);
			printf("Mailbox Size     : %lu\r\n",(long unsigned int)tChannelInfo.ulMailboxSize);
		}

		/* Read and write I/O data (32Bytes). Output data will be incremented each cycle */
		uint8_t  abSendData[32] = {0};
		uint8_t  abRecvData[32] = {0};
		uint8_t  fRunning       = 1;
		uint32_t ulState        = 0;        /* Actual state returned                    */
		uint8_t  bIdx           = 0;
		while( fRunning )
		{
			/* Wait for communication to be established */
			if( CIFX_DEV_NO_COM_FLAG == ( lRet = xChannelBusState( hChannel, CIFX_BUS_STATE_ON, &ulState, 10 ) ) )
			{
				printf("Waiting for Bus communication!\r\n");
			}
			else
			{
				/* Read Data from network */
				if(CIFX_NO_ERROR != (lRet = xChannelIORead(hChannel, 0, 0, sizeof(abRecvData), abRecvData, 10)))
				{
					printf("Error reading IO Data area!\r\n");
				} else
				{	
/*-----------------------------Edit by Leo :START -----------------------------*/
					/* Input Daten in Datei schreiben */
					FILE *fpIn;
					fpIn = fopen("input.dat", "w");
					if(fpIn == NULL) 
					{
						printf("Datei konnte nicht geoeffnet werden.\n");
					}else {
						for(int i=0; i<31; i++) {
							fprintf(fpIn, "%d\n", abRecvData[i]);
						}
						fclose(fpIn);
					}
					printf("IO Read Data:");
					DumpData(abRecvData, sizeof(abRecvData));
				
					/* Write Data to network */
					if(CIFX_NO_ERROR != (lRet = xChannelIOWrite(hChannel, 0, 0, sizeof(abSendData), abSendData, 10)))
					{
					  printf("Error writing to IO Data area!\r\n");
					} else
					{
						
						union Float2Hex offsetx ;
						union Float2Hex offsety ;
						union Float2Hex offseta ;
						/* Output Daten auf dem Bus schreiben */
						FILE *fpOut;
						fpOut = fopen("output.dat", "r");

						offsetx.fValue = 0; 
						offsety.fValue = 0; 
						offseta.fValue = 0; 
						
						float value1;
						float value2;
						float value3;
						
						if(fpOut == NULL) {
							printf("Datei konnte nicht geoeffnet werden.\n");
						}else {
							// lese Zahlen %E -> Gleitkommazahl -> float
							fscanf(fpOut, "%E\n", &value1);
							offsetx.fValue = value1;
							fscanf(fpOut, "%E\n", &value2);
							offsety.fValue = value2;
							fscanf(fpOut, "%E\n", &value3);
							offseta.fValue = value3;
							printf("Zahlen wurden gelesen.\n");
							fclose(fpOut);
						}

						printf("IO Write Data:");
						printf("\r\n");
						// zerlege den Float in 4 Byte damit diese auf den Bus adressiert werden können
						abSendData[0] = offsetx.Hex[0];
						abSendData[1] = offsetx.Hex[1];
						abSendData[2] = offsetx.Hex[2];
						abSendData[3] = offsetx.Hex[3];
						  
						abSendData[4] = offsety.Hex[0];
						abSendData[5] = offsety.Hex[1];
						abSendData[6] = offsety.Hex[2];
						abSendData[7] = offsety.Hex[3];
						  
						abSendData[8] = offseta.Hex[0];
						abSendData[9] = offseta.Hex[1];
						abSendData[10] = offseta.Hex[2];
						abSendData[11] = offseta.Hex[3];
						DumpData(abSendData, sizeof(abSendData));
					}
/*-----------------------------Edit by Leo :END-----------------------------*/
			  }
			}
			sleep( 1 );
		  }

		  xChannelClose(hChannel);
		}

    xDriverClose(hDriver);
  }
  return lRet;

}

/*****************************************************************************/
/*! Main entry function
*   \return 0                                                                */
/*****************************************************************************/
int main(int argc, char* argv[])
{
  struct CIFX_LINUX_INIT init =
  {
    .init_options        = CIFX_DRIVER_INIT_AUTOSCAN,
    .iCardNumber         = 0,
    .fEnableCardLocking  = 0,
    .base_dir            = NULL,
    .poll_interval       = 0,
    .poll_StackSize      = 0,   /* set to 0 to use default */
    .trace_level         = 255,
    .user_card_cnt       = 0,
    .user_cards          = NULL,
  };
	
#ifdef DEBUG
	printf("%s() called\n", __FUNCTION__);
#endif

  /* First of all initialize toolkit */
  int32_t lRet = cifXDriverInit(&init);

  if(CIFX_NO_ERROR == lRet)
  {

    /* Display version of cifXRTXDrv and cifXToolkit */
    DisplayDriverInformation();
  
    /* Demonstrate communication channel functionality */
    ChannelTransf();

  }

  cifXDriverDeinit();


  return 0;
}
