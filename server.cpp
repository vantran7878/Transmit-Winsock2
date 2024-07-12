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

const char* DEFAULT_PORT = "27015"; //--port for client connect
const int DEFAULT_BUFLEN = 512;

//---reads the file in binary and sends it to client in chunks

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

    //---no need for server socket
    closesocket(ListenSocket);

    //---receive and send data process
    char recvbuf[DEFAULT_BUFLEN];
    int iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;

    //---send and recv function return bytes sent or received
    //---or an error
    do
    //receive until the peer shuts down the connection
    {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            std::cout << "Bytes received: " << iResult << '\n';
            std::cout << "Message received: " << recvbuf << '\n';  

            const char* sendbuf = "Server has received";
            //--echo the buffer back to the client
            iSendResult = send(ClientSocket, sendbuf, strlen(sendbuf), 0);
            if (iSendResult == SOCKET_ERROR)
            {
                std::cout << "send failed: " << WSAGetLastError() << '\n';
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            std::cout << "Bytes sent: " << iSendResult << '\n';
        }
        else if (iResult == 0)
            std::cout << "Connection closing...\n";
        else 
        {
            std::cout << "recv failed: " << WSAGetLastError() << '\n';
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
    } 
    while (iResult > 0);

    

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