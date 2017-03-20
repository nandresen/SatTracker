#include <sstream>
#include <iostream>

#include "SerialCom.h"
#include "IpServer.h"

static int  ComRxStat = COM_RX_BUSY;
static int  ComRxErr;
static char ComRxBuf[2048];

bool isSerialComOk = false;

double currentAz;
double currentEl;

extern LogType logType;

bool SerialComSetup(char * serialComPort,long serialComBaud){

    COMMTIMEOUTS    comTimeouts;

    string logMsg;

    //  Open a handle to the specified com port.
     hCom = CreateFile( serialComPort, //name of the device to be created or opened.
                      GENERIC_READ | GENERIC_WRITE, //requested access to the file or device
                      0,      //  prevents other processes from opening a file or device if they request delete, read, or write access.
                      NULL,   //  default security attributes
                      OPEN_EXISTING, // for devices other than files - OPEN_EXISTING
                      FILE_FLAG_OVERLAPPED,      //  overlapped (asynchronous) I/O
                      NULL ); //  hTemplate, NULL for Com

    if (hCom == INVALID_HANDLE_VALUE){ //if error occurs while opening a port
       logMsg = "CreateFile failed with error: " + LastErrorString();
       Log(logMsg, logType, LOG_ERROR);
       return false;
    } //if

    if (!SetupComm(hCom, 8192, 8192)) {
        logMsg = "Could not set buffer sizes with SetupComm in SerialCom.cpp: " + LastErrorString();
        Log(logMsg, logType, LOG_ERROR);
        return false;
    } //if


    //Setting Device-Control Block (DCB) structure.
    if (!GetCommState(hCom, &dcb)) {
        logMsg = "Could not retrieve the current DCB in use for the communications port: " + LastErrorString();
        Log(logMsg, logType, LOG_ERROR);
        return false;
    } //if

    dcb.BaudRate = serialComBaud;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = TWOSTOPBITS;
    dcb.fAbortOnError= TRUE;
    //dcb.fRtsControl = RTS_CONTROL_DISABLE;
    //dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fOutxCtsFlow = FALSE;

    if (!SetCommState(hCom, &dcb)) {
        logMsg = "Could not set buffer sizes with SetupComm in SerialCom.cpp: " + LastErrorString();
        Log(logMsg, logType, LOG_ERROR);
        return false;
    } //if

    //Set communication timeouts
    if (!GetCommTimeouts (hCom, &comTimeouts)){
        logMsg = "Could not retrieve the current timeouts in use for the communications port: " + LastErrorString();
        Log(logMsg, logType, LOG_ERROR);
        return false;
    } //if

    comTimeouts.ReadIntervalTimeout = 50;
    comTimeouts.ReadTotalTimeoutConstant = 50;
    comTimeouts.ReadTotalTimeoutMultiplier = 50;
    comTimeouts.WriteTotalTimeoutConstant = 10;
    comTimeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts (hCom, &comTimeouts)) {
        logMsg = "Could not set timeout sizes with SetupComm in SerialCom.cpp: " + LastErrorString();
        Log(logMsg, logType, LOG_ERROR);
        return false;
    } //if

    ClearCommError(hCom,lpErrors,lpStat);

    Log("Serial communication function CreateFile was successful.", logType, LOG_DEBUG);
    stringstream logMsgStream;
    logMsgStream << "Serial communication parameters: " << "BaudRate - " << dcb.BaudRate << "  |  Port - " << serialComPort;
    logMsg = logMsgStream.str();
    cout << logMsg << endl;
    TxString(logMsg);
    Log(logMsg, logType, LOG_INFORMATION);
    isSerialComOk = true;
    return true;
}

//Function for sending data over serial communication
bool SerialComTx(char * data){

    OVERLAPPED  osWrite = {0};
    DWORD       dwWritten;
    DWORD       dwToWrite;
    bool        fRes;
    char        lpBuf[2048];

    string logMsg;

//    while ((ComRxStat != COM_RX_STARTED) || (ComRxStat != COM_RX_READY)) {
//        Sleep(20);
//    }

    strcpy( lpBuf, data );
    dwToWrite = strlen(lpBuf);

    // Create event writes OVERLAPPED structure hEvent.
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (osWrite.hEvent == NULL){ //If the function fails, the return value is NULL
        logMsg = "Creating overlapped event handle failed in sComTx in SerialCom.cpp: " + LastErrorString();
        Log(logMsg, logType, LOG_ERROR);
        return false;
    } //if

    // Issue write.
    if (!WriteFile(hCom, lpBuf, dwToWrite, &dwWritten, &osWrite)) {
        if (GetLastError() != ERROR_IO_PENDING) {
         //WriteFile may return before the write operation is complete.
         //In this scenario, WriteFile returns FALSE and the GetLastLOG_ERROR function returns LOG_ERROR_IO_PENDING,
         // which allows the calling process to continue while the system completes the write operation.
            logMsg = "WriteFile failed in sComTx in SerialCom.cpp ERROR_IO_PENDING: " + LastErrorString();
            Log(logMsg, logType, LOG_ERROR);
            fRes = false;
        } //if

        else {// Write is pending.
            if (!GetOverlappedResult(hCom, &osWrite, &dwWritten, TRUE)){
                logMsg = "Failed in fn GetOverlappedResult in sComTx in SerialCom.cpp: " + LastErrorString() + " Is not delayed.";
                Log(logMsg, logType, LOG_ERROR);
                fRes = false;
            } //if

            else{
                logMsg = "Write operation completed successfully after pending.";
                Log(logMsg, logType, LOG_DEBUG);
                fRes = true;
            } //else
        } //else
    } //if

    else {
        logMsg = "WriteFile completed immediately.";
        Log(logMsg, logType, LOG_DEBUG);
        fRes = true;
    }

    if (!CloseHandle(osWrite.hEvent)){
        logMsg = "CloseHandle(osWrite.hEvent) unsuccessful.";
        Log(logMsg, logType, LOG_ERROR);
    }
    CloseHandle(osWrite.hEvent);
    ComRxStat = COM_RX_RESET;
    return fRes;
}

//Function for receiving data
bool SerialComRx(){

    //DWORD dwCommEvent; //To check whether LOG_ERROR occurred while waiting event ??????
    OVERLAPPED  osReader = {0};
    BOOL        fWaitingOnRead = false;
    char        chRead; //character that has been read.
    char        bufRead[8192]; //store characters here
    DWORD       dwRead;
    DWORD       dwRes;
    DWORD       i;

    string          logMsg;
    stringstream    logMsgStream;

    int ComRxLen;
    int bufPos;

    bool ComRxRun;

    RestartComRx: //Goto label
    ComRxErr=0;
    ComRxStat=COM_RX_STARTED;

//    while (isSerialComOk != true){
//        logMsg ="Waiting for SerialComSetup.";
//        Log(logMsg, logType, LOG_DEBUG);
//        Sleep(100);
//    }

    //Check whether LOG_ERROR occurred while setting desired events that cause a notification.
    //EV_RXCHAR --> new character received and placed in the input buffer.
    if (!SetCommMask(hCom, EV_RXCHAR)){
        ComRxErr=COM_RX_MASK_ERR;
        logMsg ="Event flag EV_RXCHAR not set in SerialComRx(): " + LastErrorString();
        Log(logMsg, logType, LOG_ERROR);
        return true;
    }
    logMsg ="Event flag EV_RXCHAR set successfully in SerialComRx().";
    Log(logMsg, logType, LOG_DEBUG);

    //Create the overlapped event. Must be closed before exiting to avoid a handle leak.
    osReader.hEvent = CreateEvent(NULL, true, false, NULL);
    if (osReader.hEvent == NULL){ //error creating overlapped event
        ComRxErr = COM_RX_RD_ERR;
        logMsg ="Unsuccessful to CreateEvent in SerialComRx(): " + LastErrorString();
        Log(logMsg, logType, LOG_ERROR);
        return false;
    }
    logMsg ="CreateEvent in SerialComRx() successful.";
    Log(logMsg, logType, LOG_DEBUG);


    bufPos = 0;
    ComRxRun = true; //Ready to receive.
    while (ComRxRun) {
        logMsgStream << "Entered while (ComRxRun) in SerialComRx(). ComRxStat is: " << ComRxStat;
        logMsg = logMsgStream.str();
        Log(logMsg, logType, LOG_DEBUG);

        if(ComRxStat == COM_RX_STOP){
            if(CloseHandle(hCom) == 0){
                logMsg ="Unable to CloseHandle in SerialComRx(): " + LastErrorString();
                Log(logMsg, logType, LOG_ERROR);
            }
            ComRxStat = COM_RX_STOPPED;
            logMsg ="ComRxStat= COM_RX_STOP, ComRxStat set to COM_RX_STOPPED in SerialComRx().";
            Log(logMsg, logType, LOG_DEBUG);
            break;
        } //if

        dwRead=0;
        if (!fWaitingOnRead) { // is reading delayed?
            logMsg = "Entered >> if (!fWaitingOnRead) = false.";
            Log(logMsg, logType, LOG_DEBUG);
           // Begin read operation.
            if (!ReadFile(hCom, bufRead, 2048, &dwRead, &osReader)) {
                logMsg = "Error with ReadFile fn in SerialComRx(): " + GetLastError();
                Log(logMsg, logType, LOG_ERROR);

                if (GetLastError() != ERROR_IO_PENDING){     //reading is not delayed
                    logMsg = "Reading is not delayed, there is a serial communication error in SerialComRx(): " + GetLastError();
                    Log(logMsg, logType, LOG_ERROR);
                    logMsg = "Handles will be closed and Serial Communications is setup again in SerialComRx().";
                    Log(logMsg, logType, LOG_ERROR);
                    ComRxErr = COM_RX_RD_ERR;
                    fWaitingOnRead = false;
                    CloseHandle(hCom);
                    SerialComSetup(serialComPort,serialComBaud); //Setup serial communications again.
                    CloseHandle(osReader.hEvent);
                    goto RestartComRx;
                } //if

                else{
                    fWaitingOnRead = true;
                    logMsg = "Variable fWaitingOnRead is set to true in SerialComRx().";
                    Log(logMsg, logType, LOG_DEBUG);
                } //else
            } //if
        }//if
        else {
            string logMsg ="Reading completed immediately in SerialComRx().";
            Log(logMsg, logType, LOG_DEBUG);
        } //else

        if(fWaitingOnRead){
            logMsg = "Entered >> if (fWaitingOnRead) = true.";
            Log(logMsg, logType, LOG_DEBUG);
            dwRes = WaitForSingleObject(osReader.hEvent, 500); //STATUS_CHECK_TIMEOUT == 500
            switch(dwRes){ // Read completed.
                case WAIT_OBJECT_0:{
                    if (!GetOverlappedResult(hCom, &osReader, &dwRead, false)){
                        logMsg = "Unable to GetOverlappedResult in SerialComRx(): " + LastErrorString();
                        Log(logMsg, logType, LOG_ERROR);
                    }
                    // Read completed successfully.
                    fWaitingOnRead = false; //  Reset flag so that another operation can be issued.
                    logMsg = "Variable fWaitingOnRead is set to false in SerialComRx().";
                    Log(logMsg, logType, LOG_DEBUG);
                    break;
                } //case
                case WAIT_TIMEOUT:{
                    // Operation is not complete yet. fWaitingOnRead flag isn't
                    // changed since it will loop back around. Do not issue another read
                    //until the first one finishes.
//                    ComRxErr= COM_RX_TIMEOUT;
                    dwRead=0;
                    logMsg = "Operation ongoing dwRes = WAIT_TIMEOUT in SerialComRx().";
                    Log(logMsg, logType, LOG_DEBUG);
                    break;
                }//case
                default:{
                    string logMsg = "Error in the WaitForSingleObject in SerialComRx(): " + LastErrorString();
                    Log(logMsg, logType, LOG_ERROR);
                    ComRxErr = COM_RX_RD_ERR;
                    fWaitingOnRead = false;
//                    CloseHandle(osReader.hEvent); //Close the handle??
                    break;
                }//case
            }//switch
        }//if
        cout << "ComRxErr: " << ComRxErr << endl;
        if(!ComRxErr) {//Process the characters if LOG_ERRORs have not occurred.
//            bufPos=0;
//            ComRxBuf[0]=0;
            for(i=0; i<dwRead; i++){
                chRead = bufRead[i];
                if(ComRxStat == COM_RX_RESET){
                    string logMsg = "ComRxStat=COM_RX_RESET, bufPos & ComRxBuf[0] will be set to 0. ComRxStat will be set to COM_RX_BUSY in SerialComRx().";
                    Log(logMsg, logType, LOG_DEBUG);
                    cout << logMsg << endl;
                    bufPos = 0;
                    ComRxBuf[0] = 0;
                    ComRxStat = COM_RX_BUSY;
                } //if
                if(chRead >= ' ' ){
                    ComRxBuf[bufPos++] = chRead;
                    ComRxLen = bufPos;
    //				stringstream logMsgStream;
    //                logMsgStream << "chRead: " << chRead << endl;
    //                string logMsg = logMsgStream.str();
    //                Log(logMsg, logType, LOG_DEBUG);
    //                cout << logMsg << endl;
                } //if
                else if(chRead == '\n' ){ //line end
                    ComRxBuf[bufPos] = '\n';
                    ComRxBuf[bufPos+1] = 0;
                    ComRxLen++;
                    Log("I will set ComRxStat to COM_RX_READY.", logType, LOG_DEBUG);
                    stringstream logMsgStream;
                    logMsgStream << "ComRxBuf: " << ComRxBuf;
                    string logMsg = logMsgStream.str();
                    Log(logMsg, logType, LOG_INFORMATION);
                    cout << logMsg << endl;
                    ComRxStat = COM_RX_READY;
                    ProcessSerialFeedback(ComRxBuf);
                    while (ComRxStat == COM_RX_READY) {
                        Log("COM_RX_READY, sleep another 10ms.", logType, LOG_DEBUG);
                        Sleep(10);
                    } //while
                } //else if
            }//for --> process characters.
        }//if
    } // while ComRxRun

    if(!CloseHandle(osReader.hEvent)){
        logMsg = "CloseHandle osReader.hEvent failed in SerialComRx().";
        Log(logMsg, logType, LOG_ERROR);
        return false;
    }
    logMsg = "CloseHandle osReader.hEvent done in SerialComRx(), will return true.";
    Log(logMsg, logType, LOG_DEBUG);
    return true;
}

//Process feedback from serial and let client know.
bool ProcessSerialFeedback(char *fromSerial){

    stringstream    logMsgStream;
    string          logMsg;

    string  toClient;

    logMsgStream << "Received from the serial (stored in char *fromSerial): " << fromSerial;
    logMsg = logMsgStream.str();
    Log(logMsg, logType, LOG_INFORMATION);

    //Analyze the information from the antenna system
    cout << fromSerial << endl;
    if (fromSerial[4] == ',' ){ //5th character is comma in the standard command feedback.
        if(strncmp(fromSerial, "ECHO,", 5 ) == 0) { // 0 - the contents of both strings are equal
            if(strncmp(fromSerial, "ECHO,OK", 6 ) == 0){
                toClient = "ECHO is set.";
            }
            else if(strncmp(fromSerial, "ECHO,1", 6 ) == 0){
                toClient = "ECHO is set to 1 (ON).";
            }
            else if(strncmp(fromSerial, "ECHO,0", 6 ) == 0){
                toClient = "ECHO is set to 0 (OFF).";
            }
        }
        else if(strncmp(fromSerial, "RDEL,", 5 ) == 0) { //
            sscanf(fromSerial+5,"%le", &currentEl);
            logMsgStream << "Current elevation is: " << currentEl;
            toClient = logMsgStream.str();
        }
        else if(strncmp(fromSerial, "RDAZ,", 5 ) == 0) { //
            sscanf(fromSerial+5,"%le", &currentAz);
            logMsgStream << "Current azimuth is: " << currentAz;
            toClient = logMsgStream.str();
        } //if
//        else if(strncmp(fromSerial, "VERS,", 5 ) == 0) { //
//            sscanf(fromSerial+5,"%le", &currentVers);
//            logMsgStream << "Current version is: " << currentVers;
//            logMsg = logMsgStream.str();
//            Log(logMsg, logType, LOG_INFORMATION);
//        } //if
    }//if
    else {
        toClient = "Feedback from serial is not standard, assuming that something is wrong!";
        Log(toClient, logType, LOG_ERROR);
    }
    TxString(toClient);
    ComRxStat = COM_RX_RESET;
    return true;
}
////Process feedback from serial and let client know.
//bool ProcessSerialFeedback(char *fromSerial){
//
//    stringstream    logMsgStream;
//    string          logMsg;
//
//    string  toClient;
//
//    logMsgStream << "Received from the serial (stored in char *fromSerial): " << fromSerial;
//    logMsg = logMsgStream.str();
//    Log(logMsg, logType, LOG_INFORMATION);
//
//    //Analyze the information from the antenna system
//    cout << fromSerial << endl;
//    if (fromSerial[4] == ',' ){ //5th character is comma in the standard command feedback.
//        if(strncmp(fromSerial, "ECHO,", 5 ) == 0) { // 0 - the contents of both strings are equal
//            if(strncmp(fromSerial, "ECHO,OK", 6 ) == 0){
//                toClient = "ECHO is set.";
//            }
//            else if(strncmp(fromSerial, "ECHO,1", 6 ) == 0){
//                toClient = "ECHO is set to 1 (ON).";
//            }
//            else if(strncmp(fromSerial, "ECHO,0", 6 ) == 0){
//                toClient = "ECHO is set to 0 (OFF).";
//            }
//        }
//        else if(strncmp(fromSerial, "RDEL,", 5 ) == 0) { //
//            sscanf(fromSerial+5,"%le", &currentEl);
//            logMsgStream << "Current elevation is: " << currentEl;
//            toClient = logMsgStream.str();
//        }
//        else if(strncmp(ComRxBuf, "RDAZ,", 5 ) == 0) { //
//            sscanf(ComRxBuf+5,"%le", &currentAz);
//            logMsgStream << "Current azimuth is: " << currentAz;
//            toClient = logMsgStream.str();
//        } //if
////        else if(strncmp(fromSerial, "VERS,", 5 ) == 0) { //
////            sscanf(fromSerial+5,"%le", &currentVers);
////            logMsgStream << "Current version is: " << currentVers;
////            logMsg = logMsgStream.str();
////            Log(logMsg, logType, LOG_INFORMATION);
////        } //if
//    }//if
//
//    else {
//        toClient = "Feedback from serial is not standard, assuming that something is wrong!";
//        Log(toClient, logType, LOG_ERROR);
//    }
//
//    TxString(toClient);
//    return true;
//}



int GetComRxStat(void){
	return ComRxStat;
}

//void SetComRxStat(int status){
//	ComRxStat = status;
//}

int GetComRxErr(void){
	return ComRxErr;
}

char* GetComRxBuf(void){
	return ComRxBuf;
}

