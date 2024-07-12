#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h> //---contain most of winsock function, struct,...
#include <ws2tcpip.h> //---definitions introduced in the winsock2 protocol for TCPIP
#include <iostream>
#include <iphlpapi.h> //---use if app using IP Helper APIs, place after the winsock2.h line
#include <windows.h>
#include <fstream>
#include <thread>

//-----make the linker indicate the Ws2_32.lib file

const char* DEFAULT_PORT = "27015"; //--port for client connect
const int DEFAULT_BUFLEN = 512;

//---reads the file in binary and sends it to client in chunks
void sendFile(SOCKET ClientSocket, const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "Failed to open file: " << filename << std::endl;
        return;
    }
    char buffer[512];
    while(file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
    {
        int bytesToSend = static_cast<int>(file.gcount());
        int iResult = send(ClientSocket, buffer, bytesToSend, 0);
        if (iResult == SOCKET_ERROR)
        {
            std::cout << "send() failed: " << WSAGetLastError() << '\n';
            file.close();
            return;
        }
    }
    file.close();
}

int main()
{
    //--create WSA object 
    WSADATA wsaData;

    int iResult;
    //--- initialize winsock and check if init complete
    //--- WSAStartup function use WS2_32.dll.
    //--- MAKEWORD for WSA use the 2.2 version of winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); //result of the start up
    if (iResult != 0)
    {
        std::cout << "WSAStartup failed: " << iResult << '\n';
        return 1;
    }

    struct addrinfo *result = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; //--specify IPv4 address
    hints.ai_socktype = SOCK_STREAM; //--specify stream socket
    hints.ai_protocol = IPPROTO_TCP;//--use TCP Protocol
    hints.ai_flags = AI_PASSIVE; //--use for <bind> function
    

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        std::cout << "Getaddrinfo failed: " << iResult << '\n';
        WSACleanup();
        return 1;
    }

    SOCKET ListenSocket = INVALID_SOCKET;//server socket to listen client
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);   

    //---check is socket valid
    if (ListenSocket == INVALID_SOCKET)
    {
        std::cout << "Error at socket(): " << WSAGetLastError() << '\n';
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    //---bind the listensocket to the system client network address
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "Bind failed with error: " << WSAGetLastError() << '\n';
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    
    //---deallocated because does not need of the result
    freeaddrinfo(result);

    //---listen request information in the listensocket 
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    //---SOMAXCONN is a backlog parameter, it instruct winsock 
    //---provide the max number of connection in queue
    {
        std::cout << "Listen failed with error: " << WSAGetLastError() << '\n';
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    //---acepting the connection (single connection each time)
    SOCKET ClientSocket; //---temp client socket
    ClientSocket = INVALID_SOCKET;

    //---the accept function contain a loop to verify
    //whether to connected or not.
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        std::cout << "accept failed" << WSAGetLastError() << '\n';
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected!\n";
    //---no need for server socket
    closesocket(ListenSocket);

    //---receive and send data process

    char filename[DEFAULT_BUFLEN] = {0};
    iResult = recv(ClientSocket, filename, sizeof(filename), 0);
    if (iResult > 0)
    {
        std::cout << "Client request: " << filename << '\n';
        sendFile(ClientSocket, filename);
    }
    else
    std::cout << "recv() failed: " << WSAGetLastError() << '\n'; 

    //---disconnecting the server and client connection
    //---also shutdown the socket

    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "Shutdown failed: " << WSAGetLastError() << '\n';
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    //---clean up
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}