/*
Author:
Date:
This is the solution for the...
*/

#include <thread>

#include "Globals.h"
#include "IpServer.h"

using namespace std;

LogType logType = LOG_DEBUG; //Hiljem lisada CONF faili!

//Serial communication variables
char *serialComPort = "COM4";//com port name for example "COM1"
long serialComBaud  = 115200; // baud rate for example 115200

////TCP/IP server variables
extern SOCKADDR_IN addr; //Address that we will bind our listening socket to
extern SOCKET listenSocket;
extern SOCKET clientSocket;
extern bool isClientConnected;
extern bool isSerialComOk;

extern int gsStatus;

extern bool isTracking;
extern bool isTrackingSun;
extern bool isTrackingMoon;
extern bool isTrackingSatellite;

int main() {

    //Logger variables
    extern string   logFileDir;
    string          logMsg = " ";
    stringstream    logMsgStream;

    //Connection variables
    int serverPort = 1111; //Connection to server on port 1111
    bool serverIsPublic = true; //Broadcasted publically if true.
    int addrlen = sizeof(addr); //length of the address (required for accept call)
    string keepAlive = "Server is right by your side!";
    string toClient;

    //Create or open log file
    OpenLog(logFileDir);

     //Setup server instance
    IpServerSetup(serverPort, serverIsPublic);

    //Wait for the client
    clientSocket = accept(listenSocket, (SOCKADDR*)&addr, &addrlen);
    if (clientSocket == INVALID_SOCKET){
        Log("Failed to accept client's connection socket, clientSocket = INVALID_SOCKET.", logType, LOG_ERROR);
        cout << "Failed to accept client's connection socket." << endl;
        closesocket(listenSocket);
        WSACleanup();
    }
    else {
        Log("Accepted clientSocket!", logType, LOG_DEBUG);
        string acknowledge = "You are now connected to the server!";
        if (TxString(acknowledge)){
            logMsg = "Acknowledge message sent, we have connection with the client!";
            Log(logMsg, logType, LOG_INFORMATION);
            cout << logMsg << endl;
            isClientConnected = true;
            thread t_Network(ClientHandler);
            t_Network.detach();
        } //if
    } // else

    //Setup serial communications
    if (!SerialComSetup(serialComPort,serialComBaud)){
        toClient ="Serial communication setup failed.";
        TxString(toClient);
        Log(toClient, logType, LOG_ERROR);
    } //if
    else {
        toClient ="Serial communication setup ok, not sure about the controller.";
        TxString(toClient);
        Log(toClient, logType, LOG_INFORMATION);

        //Create threads for listening serial and TCP/IP port
        thread t_Serial(SerialComRx);
        t_Serial.detach();

        char* RxCommand = "ECHO=0\n"; //Do not need echo of the sent commands
        if (!SerialComTx(RxCommand)){ //To be sure that command is sent and that the controller is OK!
        logMsg ="Failed to send \" ECHO=0 \" to the antenna controller.";
        Log(logMsg, logType, LOG_ERROR);
        //SerialComSetup(serialComPort,serialComBaud);
        //?? RESTART SERIAL?
        }
        else {
            logMsg ="Managed to send \" ECHO=0 \" to the antenna controller. ";
            Log(logMsg, logType, LOG_INFORMATION);
        }
    } //else
    TxString(logMsg);

    int i=0;
//    while (gsStatus==BUSY) {
    while (isClientConnected == true) {
        if (isTrackingMoon == true){

            }
        Sleep(2000);
    }

//    t_Serial.~thread();
//    t_Network.~thread();

    CloseLog(); //Close the log file
	return 0;
}
