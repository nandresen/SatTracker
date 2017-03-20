#include <time.h>

#include <string>

//#include "Globals.h"

using namespace std;

string GetDateUTC(){
    time_t rawtime = time(0);   // get time now
    struct tm * nowUTC = gmtime( & rawtime );
    char dateUTC [80];
    strftime (dateUTC,80,"%Y-%m-%d-%H-%M",nowUTC);
    return dateUTC;
}

string GetTimeUTC(){
    time_t rawtime = time(0);   // get time now
    struct tm * nowUTC = gmtime( & rawtime );
    char timeUTC [80];
    strftime (timeUTC,80,"%H:%M:%S",nowUTC);
    return timeUTC;
}
