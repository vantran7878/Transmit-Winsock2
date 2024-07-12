#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h> //---contain most of winsock function, struct,...
#include <ws2tcpip.h> //---definitions introduced in the winsock2 protocol for TCPIP
#include <iostream>
#include <iphlpapi.h> //---use if app using IP Helper APIs, place after the winsock2.h line
#include <windows.h>
#include <fstream>

//-----make the linker indicate the Ws2_32.lib file
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

const char* DEFAULT_PORT = "27015"; //--port for client connect
const int DEFAULT_BUFLEN = 512;

void receiveFile(SOCKET ConnectSocket, const char* filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "Failed to open file: " << filename << '\n';
        return;
    }

    char buffer[512];
    int iResult;
    do
    {
        iResult = recv(ConnectSocket, buffer, sizeof(buffer), 0);
        if (iResult > 0)
            file.write(buffer, iResult);
        else if (iResult == 0)
            std::cout << "Connection closed\n";
        else 
            std::cout << "recv() failed" << WSAGetLastError() << '\n';
    } 
    while (iResult > 0);
    file.close(); 
}

//client.exe <server IP address>
int main(int argc, char** argv)
{
    //--create WSA object 
    WSADATA wsaData;

    int iResult;
    //--- initialize winsock and check if init complete
    //--- WSAStartup function use WS2_32.dll.
    //--- MAKEWORD for WSA use the 2.2 version of winsock

    if (argc != 3)
    {
        std::cout << "wrong command line\n";
        std::cout << "<*.exe> <Server IP address> <*.mp3>";
        return 1;
    }

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); //result of the start up
    if (iResult != 0)
    {
        std::cout << "WSAStartup failed: " << iResult << '\n';
        return 1;
    }

    struct addrinfo *result = NULL, *ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC; //--specify IPv4 address
    hints.ai_socktype = SOCK_STREAM; //--specify stream socket
    hints.ai_protocol = IPPROTO_TCP;//--use TCP Protocol

    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        std::cout << "Getaddrinfo failed: " << iResult << '\n';
        WSACleanup();
        return 1;
    }

    SOCKET ConnectSocket = INVALID_SOCKET;//server socket to listen client
    ptr = result;
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);   

    //---attempt to connect to the first address returned by
    //---the call to getaddrinfo
    if (ConnectSocket == INVALID_SOCKET)
    {
        std::cout << "Error at socket(): " << WSAGetLastError() << '\n';
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }


    //---connect server
    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "Can't connect to server!\n";
        closesocket(ConnectSocket);
        freeaddrinfo(result);
        WSACleanup();
        ConnectSocket = INVALID_SOCKET;
        return 1;
    }

    const char* filename = argv[2];

    iResult = send(ConnectSocket, filename, (int) strlen(filename), 0);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "send failed: " << WSAGetLastError() << '\n';
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    //Should really try the next address returned by getaddrinfo
    //if the connect call failed
    //But for this simple example we just free the resources
    //returned by getaddrinfo and print an error message

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        std::cout << "Server unavailable!\n";
        WSACleanup();
        return 1;
    }

    receiveFile(ConnectSocket, filename);

    closesocket(ConnectSocket);
    WSACleanup();
    //---receive and send data process
    return 0;
}