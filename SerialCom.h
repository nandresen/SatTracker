#ifndef SERIALCOM_H
#define SERIALCOM_H

#include "windows.h"

#include <sstream>
#include <string>

#include "Globals.h"

//Serial communication variables
extern char    *serialComPort;
extern long     serialComBaud;

HANDLE      hCom; //com port fileComRxRun = true;
DCB         dcb; // device-control block (DCB) structure
LPDWORD     lpErrors;
LPCOMSTAT   lpStat;

bool SerialComSetup(char *serialComPort, long serialComBaud);
bool SerialComTx(char *data);
bool SerialComRx();
bool ProcessSerialFeedback(char *ComRxBuf);
//bool SerialComSetBaudRate(long serialComBaud);

int GetComRxStat(void);
//void SetComRxStat(int status);
//int GetComRxErr(void);
char* GetComRxBuf(void);
//bool ClearComRxBuf(void);
//void ComRxStop(void);


#endif // SERIALCOM_H
