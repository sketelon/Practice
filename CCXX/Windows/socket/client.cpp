#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

//#define _DIRECT_

int main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    char buf[DEFAULT_BUFLEN] = { 0 };
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        return -1;
    }

#ifndef _DIRECT_
    struct addrinfo* result = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char* s = argv[1];
    iResult = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (iResult != 0) {
        goto _EXIT;
    }

    ConnectSocket = socket(result->ai_family, result->ai_socktype,
        result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        goto _EXIT;
    }

    iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        goto _EXIT;
    }

    freeaddrinfo(result);
#else
    ULONG uIpAddr = 0;
    WORD wPort = atoi(argv[2]);
    InetPton(AF_INET, argv[1], &uIpAddr);

    SOCKADDR_IN sai;
    sai.sin_family = AF_INET;
    sai.sin_addr.S_un.S_addr = uIpAddr;
    sai.sin_port = htons(wPort);

    ConnectSocket = socket(AF_UNSPEC, SOCK_STREAM,
        IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        goto _EXIT;
    }

    iResult = connect(ConnectSocket, (SOCKADDR*)&sai, sizeof(SOCKADDR));
    if (iResult == SOCKET_ERROR) {
        goto _EXIT;
    }

#endif


    printf("#[%s] %s open:\r\n", argv[1], argv[2]);
    do {

        gets_s(buf);
        iResult = send(ConnectSocket, buf, (int)strlen(buf) + 1, 0);
        if (iResult == SOCKET_ERROR) {
            goto _EXIT;
        }
        

        iResult = recv(ConnectSocket, buf, sizeof(buf), 0);
        printf("%s\n", buf);
    } while (iResult > 0);


_EXIT:
    if (INVALID_SOCKET != ConnectSocket) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }
    WSACleanup();

    return 0;
}
