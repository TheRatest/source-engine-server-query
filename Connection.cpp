#include "Connection.h"

void Connection::InitWSA() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
		int iErr = WSAGetLastError();
		printf("An error occured while starting up winsock (%i)\n", iErr);
		CrashApplication();
		return;
	}
}

void Connection::MakeSocket() {
	sQuery = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sQuery == INVALID_SOCKET) {
		int iErr = WSAGetLastError();
		printf("An error occured while trying to make a socket (%i)\n", iErr);
		CrashApplication();
		return;
	}
}

void Connection::SetAddress(std::string szIP, std::string szPort) {
	in_addr addr;
	if (inet_pton(AF_INET, szIP.c_str(), &addr) != 1) {
		int iErr = WSAGetLastError();
		printf("Failed to resolve IP address (%i)\n", iErr);
		CrashApplication();
		return;
	}

	saAddr.sin_addr = addr;
	saAddr.sin_port = htons(std::stoi(szPort));
	saAddr.sin_family = AF_INET;
}

void Connection::Connect() {
	printf("Connecting to %i.%i.%i.%i:%i\n", saAddr.sin_addr.S_un.S_un_b.s_b1, saAddr.sin_addr.S_un.S_un_b.s_b2, saAddr.sin_addr.S_un.S_un_b.s_b3, saAddr.sin_addr.S_un.S_un_b.s_b4, ntohs(saAddr.sin_port));
	if (connect(sQuery, (sockaddr*)&saAddr, sizeof(sockaddr_in))) {
		int iErr = WSAGetLastError();
		printf("Failed to connect to the specified address (%i)\n", iErr);
		CrashApplication();
		return;
	}
}

void Connection::Send(char* szMessage, int iLen) {
	if (send(sQuery, szMessage, iLen, 0) == 0) {
		int iErr = WSAGetLastError();
		printf("Failed to send a packet (%i)\n", iErr);
		CrashApplication();
		return;
	}
}

void Connection::Receive(char* pszBuf, int iLen) {
	/*if (iLen > 1248) {
		printf("Trying to receive a packet that is too large!\n");
		CrashApplication();
		return;
	}*/
	char szBuf[1500];
	ZeroMemory((LPVOID)szBuf, 1500);
	if (recv(sQuery, szBuf, iLen, 0) == 0) {
		int iErr = WSAGetLastError();
		printf("Failed to receive a packet (%i)\n", iErr);
		CrashApplication();
		return;
	}
	memcpy(pszBuf, szBuf, iLen);
	memcpy(szLastReceivedPacket, szBuf, 127);
	szLastReceivedPacket[127] = 0;
}

void Connection::SolveChallenge(char* szMessage, int iLength) {
	size_t iQueryEnd = iLength + 1;
	int* piChallengeNumber = reinterpret_cast<int*>(szMessage + iQueryEnd);
	memcpy_s(piChallengeNumber, 4, szLastReceivedPacket + 5, 4);

	Send(szMessage, iLength + 1 + 4);
}

void Connection::PrintMessage(char* pMsg, int iLen) {
	for (int i = 0; i < iLen; ++i) {
		std::cout << pMsg[i];
	}
	std::cout << '\n';
}

void Connection::CrashApplication() {
	Shutdown();
	exit(1);
}

void Connection::Shutdown() {
	shutdown(sQuery, SD_BOTH);
	closesocket(sQuery);
	WSACleanup();
}