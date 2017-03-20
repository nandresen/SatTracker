#include <mutex>

#include "Globals.h"

using namespace std;

extern LogType logType;

ofstream    logFile;
string      logFileDir = "./logs/";

//Open log file for logging
bool OpenLog(string logFileDir) {
    string logFileDate = GetDateUTC();
    string logFilePath = (logFileDir + logFileDate  + ".log");
    logFile.open(logFilePath.c_str(), ios::app);
    if (logFile.fail()) { // check if file is opened successfully
        string logMsg = "Opening the log file failed with an error: \n " + LastErrorString();
        cout << logMsg << endl;
        return false;
    }
    else { //file opening successful;
        string logMsg = "Opening the log file was successful.";
        cout << logMsg << endl;
        logFile << "------------------------------------------------------------------------------------"<< endl;
        return true;
    }

}

//Logging function on different severity levels.
void Log(string logMsg, LogType logType, LogType msgLogType){
//    mutex logMutex;
//    lock_guard<mutex> guard(logMutex);
    if ( logType >= msgLogType) { //log only if logType is greater.
        string logTime = GetTimeUTC();
        stringstream logMsgStream;
        logMsgStream << "<<" << msgLogType << ">> UTC: " << logTime << "  >>  " << logMsg;
        logMsg = logMsgStream.str();
//        logFile << "UTC: " + logTime + "  >>  " + logMsg << endl;
        logFile << logMsg << endl;
    } //if
    if ( logType <= 2 ) {
        cout << logMsg << endl;
    }
} //fn

//Get last Windows error as a string
string LastErrorString(){
  DWORD error = GetLastError();
  if (error)
  {
    LPVOID MsgBuf;
    DWORD bufferLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
        (LPTSTR) &MsgBuf,
        0, NULL );
    if (bufferLen)
    {
        LPCSTR MsgStr = (LPCSTR)MsgBuf;
        string errorMessage(MsgStr, MsgStr+bufferLen);

        LocalFree(MsgBuf); //cleanup
        Log(errorMessage, logType, LOG_ERROR);
        return errorMessage;
    }
  }
  else {
    string logMsg = "Could not find last error.";
    Log(logMsg, logType, LOG_ERROR);
    cout << logMsg << endl;
    return string(); //Return empty string if error not found.
  }
}

//Close log file
void CloseLog() {
    logFile.close();
}


