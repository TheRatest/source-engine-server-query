#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

class Connection
{
public:
	void InitWSA();
	void MakeSocket();
	void SetAddress(std::string szIP, std::string szPort);
	void Connect();
	void Send(char* szMessage, int iLength);
	void SolveChallenge(char* szMessage, int iLength);
	void Receive(char* pszBuf, int iLength);
	void PrintMessage(char* pMsg, int iLength);
	void Shutdown();
private:
	sockaddr_in saAddr;
	SOCKET sQuery;

	char szLastReceivedPacket[128];

	void CrashApplication();
};