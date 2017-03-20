#include <iostream>
#include <string>
#include <sstream>

#include "IpServer.h"

using namespace std;

extern LogType logType;

//TCP/IP server variables
SOCKADDR_IN addr; //Address that we will bind our listening socket to
SOCKET listenSocket = INVALID_SOCKET;
SOCKET clientSocket = INVALID_SOCKET;
bool isClientConnected = false;

//int gsStatus = AVAILABLE;

extern char *serialComPort; //??
extern long serialComBaud; //??

bool isTracking = false;
bool isTrackingSun = false;
bool isTrackingMoon = false;
bool isTrackingSatellite = false;

bool IpServerSetup(int serverPort, bool serverIsPublic){

	WSAData wsaData;

    //Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){ //If WSAStartup returns anything other than 0, then that means an "LOG_ERROR" has occured in the WinSock Startup.
		string logMsg = "WinSock startup failed.";
        cout << logMsg << endl;
        Log(logMsg, logType, LOG_ERROR);
        WSACleanup();
        return false;
	}

    if(serverIsPublic){
        addr.sin_addr.s_addr =htonl(INADDR_ANY);
        Log("Server is open to public.", logType, LOG_INFORMATION);
        }
    else{
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        Log("Server is broadcasted locally.", logType, LOG_INFORMATION);
        }

	addr.sin_port = htons(serverPort); //Port
	addr.sin_family = AF_INET; //IPv4 Socket

    //Create a socket for connecting to the server
	listenSocket = socket(AF_INET, SOCK_STREAM, 6);
	if (listenSocket == SOCKET_ERROR) {
        string logMsg = "ListenSocket creation failed.";
        cout << logMsg << endl;
        Log(logMsg, logType, LOG_ERROR);
        WSACleanup();
        return false;
	}

    // Setup the TCP listening socket
    if (bind(listenSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR){
        string logMsg = "Failed to bind the address to our socket listenSocket.";
        cout << logMsg << endl;
        Log(logMsg, logType, LOG_ERROR);
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {//Places listenSocket socket in a state in which it is listening for an incoming connection. Note:SOMAXCONN = Socket Oustanding Max connections
        string logMsg = "Failed to listen on socket listenSocket.";
        cout << logMsg << endl;
        Log(logMsg, logType, LOG_ERROR);
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }
    Log("TCP/IP server created successfully.", logType, LOG_INFORMATION);
    return true;
}

void ClientHandler(){

    stringstream    logMsgStream;
    string          logMsg;

    string fromClient;
    string toClient;

    char *RxCommand = "";

//    while (isClientConnected != true){
//        logMsg ="Waiting for client to connect before continue.";
//        Log(logMsg, logType, LOG_DEBUG);
//        Sleep(100);
//    }

    //Process client's packets while there are any.
    while (true){
        if(!RxString(fromClient)) {
            logMsg = "Unable to get message from the client.";
            Log(logMsg, logType, LOG_ERROR);
            cout << logMsg << endl;
            break;
        }
        else {
            if (fromClient.compare("HALT")== 0) {
                RxCommand = "HALT\n";
                SerialComTx(RxCommand);
            } //else if
            else if (fromClient.compare("ECHO?")== 0) {
                RxCommand = "ECHO\n";
                SerialComTx(RxCommand);
            } //else if
            else if (fromClient.compare("SET ECHO 0")== 0) {
                RxCommand = "ECHO=0\n";
                SerialComTx(RxCommand);
            } //else if
//            else if (fromClient.compare("SET ECHO 1")== 0) {
//                RxCommand = "ECHO=1\n";
//                SerialComTx(RxCommand);
//            } //else if
            else if (fromClient.compare("RDAZ")== 0) {
                RxCommand = "RDAZ\n";
                SerialComTx(RxCommand);
            } //else if
            else if (fromClient.compare("RDEL")== 0) {
                RxCommand = "RDEL\n";
                SerialComTx(RxCommand);
            } //else if
            else if (fromClient.compare("GOAZ=")== 0) {
                RxCommand = "GOAZ=24500\n"; //?? for example
                SerialComTx(RxCommand);
            } //else if
            else if (fromClient.compare("GOEL=")== 0) {
                RxCommand = "GOEL=100\n"; //?? for example
                SerialComTx(RxCommand);
            } //else if
            else if (fromClient.compare("TRACK SUN")== 0) {
                //Start tracking Sun
            } //else if
            else if (fromClient.compare("TRACK MOON")== 0) {
                //Start tracking Moon
            } //else if
            else if (fromClient.compare("TRACK SATELLITE XXXX")== 0) {
                //Start tracking Satellite
            } //else if
            else {
                toClient = "Unknown command!";
                TxString(toClient);
                Log(toClient, logType, LOG_ERROR);
            } //else

            logMsgStream << "Processed message \" " << fromClient <<" \" from client. " << endl;
            logMsg = logMsgStream.str();
            Log(logMsg, logType, LOG_DEBUG);
            cout << logMsg << endl;
            logMsgStream.str("");
        } //else
    } //while
    isClientConnected = false;
    logMsg = "Lost connection to the client. ";
    Log(logMsg, logType, LOG_ERROR);
    cout << logMsg << endl;
    Log("Will close the socket.", logType, LOG_DEBUG);
    closesocket(clientSocket); //Closes the socket if connection to client failed.
    closesocket(listenSocket);
    WSACleanup();
}

//bool ProcessClientsPacket(){
//
//    stringstream logMsgStream;
//    string logMsg;
//
//    string fromClient;
//    string toClient;
//
//    char *RxCommand = "";
//
//    if(!RxString(fromClient)) {
//        Log("Unable to get message from the client.", logType, LOG_ERROR);
//           return false;
//        } //if
//    else {
//        logMsgStream << "Processed message \" " << fromClient <<" \" from client. " << endl;
//        logMsg = logMsgStream.str();
//        Log(logMsg, logType, LOG_DEBUG);
//    } //else
//
//    cout << fromClient << endl;
//    if (fromClient.compare("STOP SERIAL") == 0) { //???
//      gsStatus = AVAILABLE;
//    }
//    else if (fromClient.compare("RESTART SERIAL") == 0) {
//        if (!SerialComSetup(serialComPort,serialComBaud)){
//        toClient ="Serial communication setup failed.";
//        Log(toClient, logType, LOG_ERROR);
//        } //if
//        else {
//            toClient ="Serial communication setup OK, not sure about the controller.";
//            Log(toClient, logType, LOG_INFORMATION);
//        } //else
//    } //else if
//    else if (fromClient.compare("HALT")== 0) {
//        RxCommand = "HALT\n";
//        SerialComTx(RxCommand);
//    } //else if
//    else if (fromClient.compare("ECHO?")== 0) {
//        RxCommand = "ECHO\n";
//        SerialComTx(RxCommand);
//    } //else if
//    else if (fromClient.compare("SET ECHO 0")== 0) {
//        RxCommand = "ECHO=0\n";
//        SerialComTx(RxCommand);
//    } //else if
//    else if (fromClient.compare("SET ECHO 1")== 0) {
//        RxCommand = "ECHO=1\n";
//        SerialComTx(RxCommand);
//    } //else if
//    else if (fromClient.compare("RDAZ")== 0) {
//        RxCommand = "RDAZ\n";
//        SerialComTx(RxCommand);
//    } //else if
//    else if (fromClient.compare("RDEL")== 0) {
//        RxCommand = "RDAZ\n";
//        SerialComTx(RxCommand);
//    } //else if
//    else if (fromClient.compare("GOAZ")== 0) {
//        RxCommand = "GOAZ=25600\n";
//        SerialComTx(RxCommand);
//    } //else if
//    else {
//        toClient = "Unknown command!";
//        TxString(toClient);
//        Log(toClient, logType, LOG_ERROR);
//    } //else
//    return true;
//}

//Functions to check whether we have received/sent all bytes
bool isRxComplete(char * data, int allBytes){
     int bytesRecv = 0;
     while (bytesRecv < allBytes){
        int checking = recv(clientSocket, data + bytesRecv, allBytes - bytesRecv, NULL);
        if (checking == SOCKET_ERROR){
            string logMsg = "Socket error in isRxComplete()." ;
            Log(logMsg, logType, LOG_ERROR);
            return false;
        }
        bytesRecv += checking;
     }
     return true;
 }

bool isTxComplete(char * data, int allBytes){
     int bytesSent = 0;
     while (bytesSent < allBytes){
        int checking = send(clientSocket, data + bytesSent, allBytes - bytesSent, NULL);
        if (checking == SOCKET_ERROR){ //If connection error occurs
            string logMsg = "Socket error in isTxComplete(). ";
            Log(logMsg, logType, LOG_ERROR);
            return false;
        }
        bytesSent += checking;
     }
     return true;
 }

//Send functions int, string
bool TxInt (int _int){
    _int = htonl(_int); //HostByteOrder to NetworkByteOrder
    if (!isTxComplete((char*)&_int, sizeof(int))){
        string logMsg = "Failed to send int to the client.";
        Log(logMsg, logType, LOG_ERROR);
        return false;
    }
    return true;
}

bool TxString (string &_string){
    int bufferLength = _string.size();
    if (!TxInt(bufferLength)){
        string logMsg = "Failed to send string length to the client.";
        Log(logMsg, logType, LOG_ERROR);
        return false;
    }
    if(!isTxComplete((char*)_string.c_str(), bufferLength)){
        string logMsg = "Failed to send string length to the client.";
        Log(logMsg, logType, LOG_ERROR);
        return false;
    }
    return true;
}

//Receive functions int, string
bool RxInt (int &_int){
    if (!isRxComplete((char*)&_int, sizeof(int))) {
        string logMsg = "Failed to receive int from the client.";
        Log(logMsg, logType, LOG_ERROR);
        return false;
    }
    _int = ntohl(_int); //NetworkByteOrder to HostByteOrder
    return true;
}

bool RxString (string &_string){
    int bufferLength;
    if (!RxInt(bufferLength)){
        string logMsg = "Failed to receive string length from the client.";
        Log(logMsg, logType, LOG_ERROR);
        return false;
    }
    char * buffer = new char[bufferLength+1]; //Create buffer
    buffer[bufferLength]= '\0'; //Set null terminator as last element
    if(!isRxComplete(buffer, bufferLength)){
        string logMsg = "Failed to receive string from the client.";
        Log(logMsg, logType, LOG_ERROR);
        delete[] buffer;
        return false;
    }
    _string = buffer;
    delete[] buffer;
    return true;
}

