#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "23"
#define DEFAULT_BUFLEN 512

int main(int argc, char** argv) {
	bool nomessage = false;
	if (argc < 4) {
		std::cout << "Not enough arguments" << std::endl;
		std::cout << "Arguments are as follows: server name, port(default telnet port is 23), message(use -nomessage for no message)";
		return 1;
	}
	if (strcmp(argv[3], "-nomessage") == 0) {
		nomessage = true;
	}
	if (nomessage == true) {
		std::cout << "Warning: Not sending message." << std::endl;
	}
	
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	iResult = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	SOCKET ConnectSocket = INVALID_SOCKET;
	ptr = result;
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	int recvbuflen = DEFAULT_BUFLEN;
	const char* sendbuf = argv[3];
	const char* empty = "";
	char recvbuf[DEFAULT_BUFLEN];
	if (nomessage == true) {
		iResult = send(ConnectSocket, empty, 0, 0);
	}
	else {
		iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
	}
	
	if (iResult == SOCKET_ERROR) {
		printf("send failed %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	printf("Bytes sent %ld\n", iResult);
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	do {
		if (iResult > 512) {
			std::cout << "Response too big." << std::endl;
			return 1;
		}
		else {
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		}
		if (iResult > 0) {
			printf("Bytes received %d\n", iResult);
			for (int i = 0; i < 512; i++) {
				if (recvbuf[i - 1] != '*') {
					std::cout << recvbuf[i];
				}
				else {
					std::cout << std::endl;
					break;
				}
			}
		}

		else if (iResult == 0) {
			printf("Connetion closed\n");
		}

		else {
			printf("recv failed: %d\n", WSAGetLastError());
		}
	} while (iResult > 0);
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}