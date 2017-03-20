#ifndef IPSERVER_H
#define IPSERVER_H

#pragma once //?
#define _WINSOCK_DEPRECATED_NO_WARNINGS //?
#pragma comment(lib,"ws2_32.lib")

#include <WinSock2.h>
#include "Globals.h"

bool IpServerSetup(int port, bool broadcastPublic = false);
//bool ProcessClientsPacket();
void ClientHandler();

//TCP/IP functions to change data
bool isTxComplete(char *data, int allBytes);
bool isRxComplete(char *data, int allBytes);

bool TxInt(int _int);
bool TxString(string &_string);

bool RxInt(int &_int);
bool RxString(string &_string);

#endif // IPSERVER_H

