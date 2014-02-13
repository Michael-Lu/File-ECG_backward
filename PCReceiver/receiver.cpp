#define WIN32_LEAN_AND_MEAN

/* Parameter that should be consistent with constant.h in project ECG */
#define PERIOD_NUM 10
#define NORMALIZED_PERIOD_LENGTH 288
#define PERIOD_TYPE u_long //The sizeof(PERIOD_TYPE) must be the same as sizeof(u_long)
/*-------------*/

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <fstream>
#include <iostream>
#include <time.h>



using namespace std;


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN (NORMALIZED_PERIOD_LENGTH*PERIOD_NUM+PERIOD_NUM)*5
#define DEFAULT_PORT "4567"

int __cdecl main(int argc, char* argv[]) 
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    struct sockaddr_in cli_sockaddr_in;
    int cli_sockaddr_len = sizeof(cli_sockaddr_in);
    char* ipstr=NULL;
    
    time_t rawtime;


    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

	char port[6];
	if(argc == 1){
		strcpy(port,DEFAULT_PORT);
		assert(!strcmp(port,DEFAULT_PORT));
		printf("default port is %s\n", port);
	}else{
		strcpy(port,argv[1]);
		printf("set port as %s\n", port);
	}



/*
    time (&rawtime);
    cout<<"Received Data at "<<ctime(&rawtime );
*/

    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }




    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();

        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();

        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();

        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();

        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();

        return 1;
    }

    if( getpeername(ClientSocket, (struct sockaddr*)&cli_sockaddr_in, &cli_sockaddr_len) !=0){
	   printf("getpeername error!\n");
    }else{
	   ipstr=inet_ntoa((&cli_sockaddr_in)->sin_addr);
	   printf("connection from %s:%d\n", ipstr, ntohs((&cli_sockaddr_in)->sin_port) );
    }

    // No longer need server socket
    closesocket(ListenSocket);


	//Create File
	ofstream ECG_data( ".\\ECG.jp2", ios::out | ios::binary);
	ofstream period_length(".\\period_length.txt", ios::out);
	
	while(1){
		printf("receiving period lengths...\n");

		int recv_bytes = 0; //count the number of received bytes when receiving period length

		ZeroMemory(recvbuf, recvbuflen);
		while(recv_bytes < PERIOD_NUM * sizeof(PERIOD_TYPE)){

			iResult = recv(ClientSocket, recvbuf+recv_bytes, (PERIOD_NUM * sizeof(PERIOD_TYPE) - recv_bytes), 0);
			if (iResult > 0) {
				recv_bytes += iResult;
				printf("bytes received: %d\n", recv_bytes);
			}
			else if (iResult == 0){
				printf("Connection closing...\n");
				goto DONE;
			}else{
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				period_length.close();
				ECG_data.close();

				return 1;
			}
		}

		PERIOD_TYPE* pRecvPeriod = (PERIOD_TYPE*)recvbuf;
		for(short i=0; i<PERIOD_NUM ; i++)
			period_length<<ntohl(pRecvPeriod[i])<<endl; //write the received periods to the file

		// Receive the length of jp2 image length
		printf("receiving jp2 image length...\n");

		ZeroMemory(recvbuf, recvbuflen);
		recv_bytes = 0;
		while(recv_bytes < sizeof(PERIOD_TYPE) ){

			iResult = recv(ClientSocket, recvbuf+recv_bytes, (sizeof(PERIOD_TYPE)-recv_bytes), 0);
			if (iResult > 0) {
				recv_bytes += iResult;
				printf("bytes received: %d\n", recv_bytes);
			}
			else if (iResult == 0){
				printf("Connection closing...\n");
				goto DONE;
			}else{
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				period_length.close();
				ECG_data.close();

				return 1;
			}
		}

		PERIOD_TYPE jp2ImageLen = ((PERIOD_TYPE*)recvbuf)[0];
		printf("Image length is %d\n", jp2ImageLen);

		printf("receiving jp2 image...\n");
		
		recv_bytes = 0;
		while(recv_bytes < jp2ImageLen) {

			ZeroMemory(recvbuf, recvbuflen);
			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				//printf("Bytes received: %d\n", iResult);
				ECG_data.write(recvbuf,iResult);
				recv_bytes += iResult;
			}
			else if (iResult == 0){
				printf("Connection closing...\n");
				goto DONE;
			}else{
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				ECG_data.close();

				return 1;
			}
		}
		printf("received image data %d bytes\n", recv_bytes);

	}

/*
    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        ECG_data.close();

        return 1;
    }
*/
DONE:    // cleanup
    closesocket(ClientSocket);
    WSACleanup();
    ECG_data.close();
	period_length.close();


    return 0;
}
