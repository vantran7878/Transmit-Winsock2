#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h> //---contain most of winsock function, struct,...
#include <ws2tcpip.h> //---definitions introduced in the winsock2 protocol for TCPIP
#include <iostream>
#include <iphlpapi.h> //---use if app using IP Helper APIs, place after the winsock2.h line
#include <windows.h>

//-----make the linker indicate the Ws2_32.lib file
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

const char* DEFAULT_PORT = "27015"; //--port for client connect
const int DEFAULT_BUFLEN = 512;


//client.exe <server IP address>
int main(int argc, char** argv)
{
    //--create WSA object 
    WSADATA wsaData;

    int iResult;
    //--- initialize winsock and check if init complete
    //--- WSAStartup function use WS2_32.dll.
    //--- MAKEWORD for WSA use the 2.2 version of winsock

    if (argc != 2)
    {
        std::cout << "wrong command line\n";
        std::cout << "<*.exe> <Server IP address>";
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
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    //Should really try the next address returned by getaddrinfo
    //if the connect call failed
    //But for this simple example we just free the resources
    //returned by getaddrinfo and print an error message

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        std::cout << "Unable to connect to server!\n";
        WSACleanup();
        return 1;
    }

    //---receive and send data process
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    const char* sendbuf = "This is a test";

    iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "send failed: " << WSAGetLastError() << '\n';
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Bytes sent: " << iResult << '\n';

    //---shutdown the connection for sending since no more data will be sent
    //--- the client can still use the ConnectSocket for receiving data
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "Shutdown failed: " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }

    //---receive data untile the server closes the connection
    do
    {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            std::cout << "Bytes received: " << iResult << '\n';
            std::cout << "Message received: " << recvbuf << '\n';
        }
        else if (iResult == 0)
            std::cout << "Connection closed\n";
        else 
            std::cout << "Receiving failed: " << WSAGetLastError() << '\n';
    } while (iResult > 0);

    // shutdown the send half of the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND); 
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "Shutdown failed: " << WSAGetLastError() << '\n';
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    //---clean up
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}