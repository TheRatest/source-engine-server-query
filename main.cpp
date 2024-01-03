#include <iostream>
#include <cstdio>
#include "Connection.h"

#pragma comment (lib, "ws2_32.lib")

typedef struct MultiPacketHeader {
	long int lHeader;
	long int lID;
	unsigned char bTotalPackets;
	unsigned char bPacketNum;
	short iMaxPacketSize;
	// only if the packet is compressed using bzip2
	/*long int lUncompressedSize;
	long int lCRC32CheckSum;*/
} MultiPacketHeader;

typedef struct MultiPacketInfo {
	long int lID;
	unsigned char bPacketNum;
	void* pPacket;
	void* pPayload;
	int iPayloadSize;
} MultiPacketInfo;

int main(int argc, char** argv) {
	std::string szAddress = "";
	std::string szIP = "";
	std::string szPort = "";
	std::string szOutputFile = "";
	bool bShowPlayers = false;
	bool bShowRules = true;
	// CLI Args
	for (int iArg = 1; iArg < argc; ++iArg) {
		if (strcmp(argv[iArg], "-a") == 0 || strcmp(argv[iArg], "-address") == 0 || strcmp(argv[iArg], "-addr") == 0) {
			++iArg;
			szAddress = argv[iArg];
			szIP = szAddress.substr(0, szAddress.find(":"));
			szPort = szAddress.substr(szAddress.find(":") + 1, szAddress.length() - szIP.length());
			continue;
		}
		if (strcmp(argv[iArg], "-port") == 0) {
			++iArg;
			szPort = argv[iArg];
			continue;
		}
		if (strcmp(argv[iArg], "-ip") == 0) {
			++iArg;
			szIP = argv[iArg];
			continue;
		}
		if (strcmp(argv[iArg], "-p") == 0 || strcmp(argv[iArg], "-players") == 0) {
			bShowPlayers = true;
			continue;
		}
		if (strcmp(argv[iArg], "-r") == 0 || strcmp(argv[iArg], "-rules") == 0) {
			bShowRules = true;
			continue;
		}
		if (strcmp(argv[iArg], "-o") == 0 || strcmp(argv[iArg], "-out") == 0 || strcmp(argv[iArg], "-output") == 0) {
			++iArg;
			szOutputFile = argv[iArg];
			continue;
		}
	}
	if (szPort == "")
		szPort = "27015";

	if (szIP == "") {
		printf("Enter server ip & port (ip:port): ");
		std::getline(std::cin, szAddress);
		szIP = szAddress.substr(0, szAddress.find(":"));
		szPort = szAddress.substr(szAddress.find(":") + 1, szAddress.length() - szIP.length());
	}

	FILE* fStream = stdout;
	if (szOutputFile != "") {
		freopen_s(&fStream, szOutputFile.c_str(), "w", stdout);
	}

	Connection cnQuery;
	cnQuery.InitWSA();
	cnQuery.MakeSocket();
	cnQuery.SetAddress(szIP, szPort);
	cnQuery.Connect();
	
	char szInitialQuery[64] = "\xFF\xFF\xFF\xFF\x54Source Engine Query";
	cnQuery.Send(szInitialQuery, strlen(szInitialQuery) + 1);

	char* pResponse = new char[1500];
	cnQuery.Receive(pResponse, 1500);
	
	// server requires challenge completion
	if (pResponse[4] == 'A') {
		cnQuery.SolveChallenge(szInitialQuery, strlen(szInitialQuery));
		cnQuery.Receive(pResponse, 1500);
	}

	// got valid respone
	if (pResponse[4] == 'I') {
		size_t iCursor = 5;
		char bProtocol = pResponse[iCursor]; iCursor += sizeof(char);
		char szServerName[256]; strcpy_s(szServerName, 256, pResponse + iCursor); iCursor += strlen(szServerName) + 1;
		char szMapName[128]; strcpy_s(szMapName, 128, pResponse + iCursor); iCursor += strlen(szMapName) + 1;
		char szFolder[128]; strcpy_s(szFolder, 128, pResponse + iCursor); iCursor += strlen(szFolder) + 1;
		char szGame[128]; strcpy_s(szGame, 128, pResponse + iCursor); iCursor += strlen(szGame) + 1;
		short iGameID = *reinterpret_cast<short*>(pResponse + iCursor); iCursor += sizeof(short);
		char bPlayers = pResponse[iCursor]; iCursor += sizeof(char);
		char bMaxPlayers = pResponse[iCursor]; iCursor += sizeof(char);
		char bBots = pResponse[iCursor]; iCursor += sizeof(char);
		char bServerType = pResponse[iCursor]; iCursor += sizeof(char);
		char bServerOS = pResponse[iCursor]; iCursor += sizeof(char);
		bool bPassword = pResponse[iCursor]; iCursor += sizeof(char);
		bool bVAC = pResponse[iCursor]; iCursor += sizeof(char);
		char szVersion[128]; strcpy_s(szVersion, 128, pResponse + iCursor); iCursor += strlen(szVersion) + 1;
		char bExtraDataFlags = pResponse[iCursor]; iCursor += sizeof(char);
		// extra stuff, may not be there
		short iServerPort = 0;
		long long llSteamId = 0;
		short iSourceTVPort = 0;
		char szSourceTVName[256];
		ZeroMemory(szSourceTVName, 128);
		char szTags[256];
		ZeroMemory(szTags, 256);
		long long llGameId = 0;
		
		if (bExtraDataFlags & 0x80) {
			iServerPort = *reinterpret_cast<short*>(pResponse + iCursor); iCursor += sizeof(short);
		}
		if (bExtraDataFlags & 0x10) {
			llSteamId = *reinterpret_cast<long long*>(pResponse + iCursor); iCursor += sizeof(long long);
		}
		if (bExtraDataFlags & 0x40) {
			iSourceTVPort = *reinterpret_cast<short*>(pResponse + iCursor); iCursor += sizeof(short);
			strcpy_s(szSourceTVName, 256, pResponse + iCursor); iCursor += strlen(szSourceTVName) + 1;
		}
		if (bExtraDataFlags & 0x20) {
			strcpy_s(szTags, 256, pResponse + iCursor); iCursor += strlen(szTags) + 1;
		}
		if (bExtraDataFlags & 0x01) {
			llGameId = *reinterpret_cast<long long*>(pResponse + iCursor); iCursor += sizeof(long long);
		}

		printf("Name: %s\n", szServerName);
		printf("Current Map: %s\n", szMapName);
		printf("Players: %i\n", bPlayers);
		printf("Bots: %i\n", bBots);
		printf("Max Players: %i\n", bMaxPlayers);
		printf("Game: %s\n", szGame);
		printf("Game Folder: %s\n", szFolder);
		printf("Game Version: %s\n", szVersion);
		printf("Game ID: %i\n", iGameID);
		char szServerOS[16];
		if (bServerOS == 'l')
			strcpy_s(szServerOS, 16, "Linux");
		if (bServerOS == 'w')
			strcpy_s(szServerOS, 16, "Windows");
		if (bServerOS == 'o')
			strcpy_s(szServerOS, 16, "Mac");
		char szServerType[16];
		if (bServerType == 'd')
			strcpy_s(szServerType, 16, "Dedicated");
		if (bServerType == 'l')
			strcpy_s(szServerType, 16, "Listen");
		if (bServerType == 'p')
			strcpy_s(szServerType, 16, "SourceTV Proxy");
		printf("Server OS: %s\n", szServerOS);
		printf("Server Type: %s\n", szServerType);
		char szVACEnabled[10];
		if (bVAC)
			strcpy_s(szVACEnabled, 10, "Enabled");
		else
			strcpy_s(szVACEnabled, 10, "Disabled");
		printf("VAC: %s\n", szVACEnabled);
		char szPassworded[10];
		if (bPassword)
			strcpy_s(szPassworded, 10, "Enabled");
		else
			strcpy_s(szPassworded, 10, "Disabled");
		printf("Password: %s\n", szPassworded);
		printf("Server Port: %i\n", iServerPort);
		printf("Tags: %s\n", szTags);
		printf("SourceTV Port: %i\n", iSourceTVPort);
		printf("SourceTV Name: %s\n", szSourceTVName);
		printf("SteamID: %lli\n", llSteamId);
		printf("GameID: %lli\n", llGameId);
	}

	if (bShowPlayers) {
		printf("\n");
		char szPlayersQuery[64] = "\xFF\xFF\xFF\xFF\x55\xFF\xFF\xFF\xFF";
		cnQuery.Send(szPlayersQuery, strlen(szPlayersQuery));

		cnQuery.Receive(pResponse, 1248);

		// server requires challenge completion
		if (pResponse[4] == 'A') {
			cnQuery.SolveChallenge(szPlayersQuery, strlen(szPlayersQuery) - 5);
			cnQuery.Receive(pResponse, 1248);
		}

		if (pResponse[4] != 'D') {
			printf("Failed to fetch current players!\n");
			return EXIT_FAILURE;
		}

		size_t iCursor = 5;
		char bPlayers = pResponse[iCursor]; iCursor += sizeof(char);
		if (bPlayers > 0) {
			printf("Current Players: \n");
			printf("Player Index | Name | Score | Seconds on server\n");
			for (char i = 0; i < bPlayers; ++i) {
				char bIndex = pResponse[iCursor]; iCursor += sizeof(char);
				char szName[256]; strcpy_s(szName, 256, pResponse + iCursor); iCursor += strlen(szName) + 1;
				long iScore = *reinterpret_cast<long*>(pResponse + iCursor); iCursor += sizeof(long);
				float fPlaytime = *reinterpret_cast<float*>(pResponse + iCursor); iCursor += sizeof(float);
				printf("%i | %s | %i | %f\n", bIndex, szName, iScore, fPlaytime);
			}
		}
		else {
			printf("Current Players: none\n");
		}

	}

	if (bShowRules) {
		printf("\n");
		char szRulesQuery[64] = "\xFF\xFF\xFF\xFF\x56\xFF\xFF\xFF\xFF";
		cnQuery.Send(szRulesQuery, strlen(szRulesQuery));

		cnQuery.Receive(pResponse, 1248 + sizeof(MultiPacketHeader));

		// server requires challenge completion
		if (pResponse[4] == 'A') {
			cnQuery.SolveChallenge(szRulesQuery, strlen(szRulesQuery) - 5);
			cnQuery.Receive(pResponse, 1248 + sizeof(MultiPacketHeader));
		}

		MultiPacketHeader* hdParentPacket = reinterpret_cast<MultiPacketHeader*>(pResponse);
		// if there's a single packet in the response
		if (hdParentPacket->lHeader == 0xFFFFFFFF) {
			if (pResponse[4] != 'E') {
				printf("Failed to fetch server rules!\n");
				return EXIT_FAILURE;
			}

			size_t iCursor = 5;
			short iRules = *reinterpret_cast<short*>(pResponse + iCursor); iCursor += sizeof(short);
			if (iRules > 0) {
				printf("Server Rules: \n");
				printf("Name | Value\n");
				for (char i = 0; i < iRules; ++i) {
					char szName[256]; strcpy_s(szName, 256, pResponse + iCursor); iCursor += strlen(szName) + 1;
					char szValue[256]; strcpy_s(szValue, 256, pResponse + iCursor); iCursor += strlen(szValue) + 1;
					printf("%s | %s \n", szName, szValue);
				}
			}
			else {
				printf("Server Rules: none\n");
			}
		}
		else /* if the packet is split */ if (hdParentPacket->lHeader == 0xFFFFFFFE) {
			int iPacketsReceived = 1;
			int iBigResponseSize = hdParentPacket->iMaxPacketSize * hdParentPacket->bTotalPackets;
			char* pBigResponse = new char[iBigResponseSize];
			ZeroMemory(pBigResponse, iBigResponseSize);

			MultiPacketInfo* mpiPackets = new MultiPacketInfo[hdParentPacket->bTotalPackets];

			MultiPacketInfo* piParentPacket = mpiPackets;
			piParentPacket->lID = hdParentPacket->lID;
			piParentPacket->bPacketNum = hdParentPacket->bPacketNum;
			piParentPacket->pPacket = pResponse;
			piParentPacket->pPayload = (void*)((DWORD)piParentPacket->pPacket + sizeof(MultiPacketHeader));
			piParentPacket->iPayloadSize = hdParentPacket->iMaxPacketSize;

			for (int i = 1; i < hdParentPacket->bTotalPackets; ++i) {
				char* pBuffer = new char[hdParentPacket->iMaxPacketSize + 40];
				ZeroMemory(pBuffer, hdParentPacket->iMaxPacketSize + 40);
				cnQuery.Receive(pBuffer, hdParentPacket->iMaxPacketSize + sizeof(MultiPacketHeader));

				MultiPacketHeader* hdPacket = reinterpret_cast<MultiPacketHeader*>(pBuffer);

				MultiPacketInfo* piPacket = mpiPackets + i;
				piPacket->lID = hdPacket->lID;
				piPacket->bPacketNum = hdPacket->bPacketNum;
				piPacket->pPacket = pBuffer;
				piPacket->pPayload = (void*)((DWORD)piPacket->pPacket + sizeof(MultiPacketHeader));
				piPacket->iPayloadSize = hdParentPacket->iMaxPacketSize;
			}

			size_t iPasteCursor = 0;
			for (int i = 0; i < hdParentPacket->bTotalPackets; ++i) {
				for (int j = 0; j < hdParentPacket->bTotalPackets; ++j) {
					if (mpiPackets[j].bPacketNum != i)
						continue;

					void* pDest = pBigResponse + iPasteCursor;
					memcpy_s(pDest, hdParentPacket->iMaxPacketSize, mpiPackets[j].pPayload, mpiPackets[j].iPayloadSize);
					iPasteCursor += mpiPackets[j].iPayloadSize;
				}
			}
			
			//cnQuery.PrintMessage(pBigResponse, hdParentPacket->iMaxPacketSize* hdParentPacket->bTotalPackets);
			
			size_t iCursor = 5;
			short iRules = *reinterpret_cast<short*>(pBigResponse + iCursor); iCursor += sizeof(short);
			if (iRules > 0) {
				printf("Server Rules (%i total): \n", iRules);
				printf("Name | Value\n");
				for (int i = 0; i < iRules; ++i) {
					char szName[256]; strcpy_s(szName, 256, pBigResponse + iCursor); iCursor += strlen(szName) + 1;
					char szValue[256]; strcpy_s(szValue, 256, pBigResponse + iCursor); iCursor += strlen(szValue) + 1;
					printf("%s | %s \n", szName, szValue);
				}
			}
			else {
				printf("Server Rules: none\n");
			}
		}
	}

	if (szOutputFile != "") {
		fclose(fStream);
	}

	delete[] pResponse;
	cnQuery.Shutdown();
	return EXIT_SUCCESS;
}