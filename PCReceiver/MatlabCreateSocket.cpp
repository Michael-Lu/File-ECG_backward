/*==========================================================
 * arrayProduct.c - example in MATLAB External Interfaces
 *
 * Multiplies an input scalar (multiplier) 
 * times a 1xN matrix (inMatrix)
 * and outputs a 1xN matrix (outMatrix)
 *
 * The calling syntax is:
 *
 *		outMatrix = arrayProduct(multiplier, inMatrix)
 *
 * This is a MEX-file for MATLAB.
 * Copyright 2007-2008 The MathWorks, Inc.
 *
 *========================================================*/
/* $Revision: 1.1.10.2 $ */

#include "mex.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <errno.h>

#pragma comment (lib, "Ws2_32.lib")

static SOCKET listen_on_socket(char* addr, char* port = "4567");

/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{


    WSADATA wsaData;

    if(nlhs != 2){
        mexPrintf("There should be exactly 3 output argument, [raw_data, period, err]\n");
        return;
    }

    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }


    plhs[0] = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
    int* pmxData = (int*) mxGetData(plhs[0]);
    *pmxData = ClientSocket;
    printf("Accepted Connection on Socket %d", ClientSocket);

    return;
}


static SOCKET listen_on_socket(char* addr, char* port = "4567"){
// Remember to call WSAStartup before calling this function.
// And, remember to WSAcleanup after calling this function.
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    struct sockaddr_in cli_sockaddr_in;
    int cli_sockaddr_len = sizeof(cli_sockaddr_in);
    char* ipstr=NULL;


    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
		errno = iResult;
        return INVALID_SOCKET;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
		errno = WSAGetLastError();
		
        return INVALID_SOCKET;
    }


    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        errno = WSAGetLastError();

        return INVALID_SOCKET;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        errno = WSAGetLastError();

        return INVALID_SOCKET;
    }
	printf("Listening on port %d\n", port);
    printf("Listening on socket %d....Remember to close the socket mechanically.\n", ListenSocket);

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        errno = WSAGetLastError();

        return INVALID_SOCKET;
    }

    if( getpeername(ClientSocket, (struct sockaddr*)&cli_sockaddr_in, &cli_sockaddr_len) !=0){
       printf("getpeername error!\n");
    }else{
       ipstr=inet_ntoa((&cli_sockaddr_in)->sin_addr);
       printf("connection from %s:%d\n", ipstr, ntohs((&cli_sockaddr_in)->sin_port) );
    }

    // No longer need server listening socket
    closesocket(ListenSocket);
	
	return ClientSocket;

}
