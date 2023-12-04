#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_DESTINY2_PORT 3097

SOCKET s = INVALID_SOCKET;

void cleanup(void)
{
    closesocket(s);
    WSACleanup();
}

BOOL WINAPI ConsoleHandler(DWORD dwType)
{
    UNREFERENCED_PARAMETER(dwType);
    cleanup();
    return TRUE;
}

int __cdecl wmain(int argc, wchar_t** argv)
{
    int port = DEFAULT_DESTINY2_PORT;
    const int af = AF_INET;

    if (argc == 2) {
        port = _wtoi(argv[1]);
    }

    if (argc > 2) {
        wprintf(L"usage: %ws <port>\n", argv[0]);
        wprintf(L"    <port> is the UDP port number to listen on\n");
        wprintf(L"    <port> must be greater than 0 and less than 65535\n");
        wprintf(L"Example: %ws 3097\n", argv[0]);
        return 1;
    }

    if (port <= 0 || port >= 65535) {
        wprintf(L"Port must be greater than 0 and less than 65535\n");
        return 1;
    }

    // Initialize Winsock
    WSADATA wsaData;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        wprintf(L"WSAStartup failed with error: %d\n", result);
        return 1;
    }

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);
    atexit(cleanup);

    // Create the socket
    s = socket(af, SOCK_DGRAM, IPPROTO_UDP);

    if (s == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set the exclusive address option
    const int sockOptVal = 1;
    result = setsockopt(s, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
        (char*)&sockOptVal, sizeof(sockOptVal));

    if (result == SOCKET_ERROR) {
        wprintf(L"setsockopt for SO_EXCLUSIVEADDRUSE failed with error: %ld\n",
            WSAGetLastError());
        cleanup();
        return 1;
    }

    SOCKADDR_IN saLocal;
    SecureZeroMemory(&saLocal, sizeof(saLocal));
    saLocal.sin_family = (ADDRESS_FAMILY)af;
    saLocal.sin_port = htons((u_short)port);
    saLocal.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket
    result = bind(s, (SOCKADDR*)&saLocal, sizeof(saLocal));

    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();

        if (err == WSAEACCES) {
            wprintf(L"bind failed with WSAEACCES (access denied): %ld\n", err);
        } else {
            wprintf(L"bind failed with error: %ld\n", err);
        }

        cleanup();
        return 1;
    }

    wprintf(L"Press [Enter] key to exit.\n");
    (void)_getch();

    return 0;
}