#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")


#define DEFAULT_BUFLEN 512

#define _DIRECT_

void close_socket(SOCKET& sock)
{
    if (INVALID_SOCKET != sock) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
}

int main(int argc, char** argv)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    int iSendResult;
    char buf[DEFAULT_BUFLEN] = { 0 };

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        return 0;
    }

#ifndef _DIRECT_
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, argv[1], &hints, &result);
    if (iResult != 0) {
        goto _EXIT;
    }

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        goto _EXIT;
    }


    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        freeaddrinfo(result);
        goto _EXIT;
    }

    freeaddrinfo(result);
#else

    WORD wPort = atoi(argv[1]);

    SOCKADDR_IN sai;
    sai.sin_family = AF_INET;
    sai.sin_addr.S_un.S_addr = INADDR_ANY;
    sai.sin_port = htons(wPort);

    ListenSocket = socket(AF_UNSPEC, SOCK_STREAM,
        IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        goto _EXIT;
    }


    iResult = bind(ListenSocket, (SOCKADDR*)&sai, sizeof(SOCKADDR));
    if (iResult == SOCKET_ERROR) {
        goto _EXIT;
    }

#endif

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        goto _EXIT;
    }


    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        goto _EXIT;
    }


    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, buf, sizeof(buf), 0);
        printf("%s\n", buf);
        if (iResult > 0) 
        {
            iSendResult = send(ClientSocket, buf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                goto _EXIT;
            }
        }

    } while (iResult > 0);


_EXIT:
    close_socket(ClientSocket);
    close_socket(ListenSocket);
    WSACleanup();

    return 0;
}