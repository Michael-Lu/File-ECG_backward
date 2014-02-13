#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

using namespace std;

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main() {

    //----------------------
    // Declare and initialize variables.
    WSADATA wsaData;
    int iResult;

    SOCKET ConnectSocket = INVALID_SOCKET;
    struct sockaddr_in clientService; 

    u_long sendbuf[10];
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;


    //----------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != NO_ERROR) {
      printf("WSAStartup failed: %d\n", iResult);
      return 1;
    }

    //----------------------
    // Create a SOCKET for connecting to server
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError() );
        WSACleanup();
        return 1;
    }

    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr( "140.113.13.53" );
    clientService.sin_port = htons( 4567 );

    //----------------------
    // Connect to server.
    iResult = connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) );
    if ( iResult == SOCKET_ERROR) {
        closesocket (ConnectSocket);
        printf("Unable to connect to server: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    for(short count = 0; count< 10; ){
        for(short i=0; i<10 ;i++)
            sendbuf[i] = htonl(2*count+i*10);

        iResult = send( ConnectSocket, (char*)sendbuf, 10*sizeof(u_long), 0 );
        if (iResult == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        printf("Send period lengths done: %ld bytes\n", iResult);

    	// Send the lenght of jp2Image
    	ifstream data(".\\Sunset.jp2", ios::binary);

    	//calculate the length of jp2Image
    	data.seekg(0, data.end);
    	sendbuf[0] =  data.tellg();
    	data.seekg(0, data.beg);

        iResult = send( ConnectSocket, (char*)sendbuf, sizeof(u_long), 0 );
        if (iResult == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

    	char tmpdata[1000];
    	int send_bytes = 0;

    	while(!data.eof() && data.good() && send_bytes < sendbuf[0]){
    		data.read(tmpdata,1000);
    		iResult = send( ConnectSocket, tmpdata, data.gcount(), 0 );
    		if (iResult == SOCKET_ERROR) {
    			printf("send failed: %d\n", WSAGetLastError());
    			closesocket(ConnectSocket);
    			WSACleanup();
    			return 1;
    		}else{
    			send_bytes += iResult;
    		}
    	
    	}
    	printf("send %d bytes done\n", send_bytes);

/*
    char flag;
    cin.get(flag);
    cin.ignore();
    if( flag == 'n' || flag == 'N')
        break;
*/
    count++;
    Sleep(1000);
    }
    
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}


