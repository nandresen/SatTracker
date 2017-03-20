#include <iostream>
#include <fstream>
#include <mutex>
#include "windows.h"

#include "DateTimeFns.h"

using namespace std;

enum logLevels {
    Critical = 1, //Value: 1. Indicates logs for a critical alert.
    Error, //Value: 2. Indicates logs for an error.
    Warning, //Value: 3. Indicates logs for a warning.
    Information, //Value: 4. Indicates logs for an informational message.
    Debug //Value: 5. Indicates logs at all levels for debugging.
    };

//Open log file for logging
bool OpenLog(string logFileDir);

//Logging functions on different severity levels. Critical, Error, Warning, Information, Debug
void Log(string logMessage, logLevels logLevel, logLevels msgLogLevel);

//Get last windows error as a string
string LastErrorString();

//Close log file
void CloseLog();


