#ifndef GLOBALS_H
#define GLOBALS_H

//gsStatus values
#define MAINTENANCE -1
#define AVAILABLE      0
#define BUSY       1

//ComRxStat values
#define COM_RX_BUSY		0
#define COM_RX_RESET	1
#define COM_RX_READY	2
#define COM_RX_STARTED	3
#define COM_RX_STOP		4
#define COM_RX_STOPPED	5

//ComRxErr values
#define COM_RX_RD_ERR	1
#define COM_RX_EV_ERR	2
#define COM_RX_MASK_ERR	4
#define COM_RX_TIMEOUT	8

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <WinSock2.h>

using namespace std;

enum LogType {
    LOG_CRITICAL = 1, //Value: 1. Indicates logs for a critical alert.
    LOG_ERROR, //Value: 2. Indicates logs for an error.
    LOG_WARNING_LOG, //Value: 3. Indicates logs for a warning., //Value: 3. Indicates logs for a warning.
    LOG_INFORMATION, //Value: 4. Indicates logs for an informational message.
    LOG_DEBUG //Value: 5. Indicates logs at all levels for debugging.
    };

//Logging functions
bool OpenLog(string logFileDir);
void Log(string logMsg, LogType logType, LogType msglogType);
string LastErrorString();
void CloseLog();

//Serial communications functions
bool SerialComSetup(char *serialComPort, long serialComBaud);
bool SerialComTx(char *data);
bool SerialComRx();
bool ProcessSerialFeedback(char *fromSerial);
int GetComRxStat(void);
int GetComRxErr(void);
char* GetComRxBuf(void);

//Date and Time functions
string GetDateUTC();
string GetTimeUTC();

#endif // GLOBALS_H

