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

#define PERIOD_NUM 10
#define NORMALIZED_PERIOD_LENGTH 288
#define PERIOD_TYPE u_long
#define NTOH(x) ntohl(x)

#pragma comment (lib, "Ws2_32.lib")

/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{


    //WSADATA wsaData;
    int iResult;
    SOCKET ClientSocket = INVALID_SOCKET;
    PERIOD_TYPE* period_length; //period_length should be replaced by the input pointer from Matlab working space;
   
    
#if  PERIOD_TYPE == u_long

    plhs[0] = mxCreateNumericMatrix(PERIOD_NUM, 1, mxUINT32_CLASS, mxREAL);
    period_length = (u_long*)mxGetData(plhs[0]);
    
#else
    
    printf("The PERIOD_TYPE Macro in mex file ReceiveSocket didn't be set properly.\n");
    return;
    
#endif
    
    
    if(nrhs != 1){
        mexPrintf("\n\tThere should only be 1 input argument. That is the socket file descriptor\n");
        return;
    }
    
    if(! mxIsClass(prhs[0], "uint32")){
       mexPrintf("\n\tInput should be uint32 \n"); 
       return;      
    }
    
    if(nlhs != 2){
        mexPrintf("\n\tThere should be 2 output argument: [Period_Length, JP2ECGImage]\n");
        return;
    }

    

    unsigned int* pmxData = (unsigned int*) mxGetData(prhs[0]);
    ClientSocket = *pmxData;
    printf("ClientSocket is %d\n", ClientSocket);
    
    /*
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }
     */


	printf("receiving period lengths...\n");

    int recv_bytes = 0; //count the number of received bytes when receiving period length
    
    while(recv_bytes < PERIOD_NUM * sizeof(PERIOD_TYPE)){

        iResult = recv(ClientSocket, (char*)period_length+recv_bytes, (PERIOD_NUM * sizeof(PERIOD_TYPE) - recv_bytes), 0);
        if (iResult > 0) {
            recv_bytes += iResult;
            printf("bytes received: %d\n", recv_bytes);
        }
        else if (iResult == 0){
            printf("Connection closing...\n");
            return;
        }else{
            printf("recv failed with error: %d\n", WSAGetLastError());
            return;
        }
    }

    for(short i=0; i<PERIOD_NUM ; i++)
        period_length[i] = NTOH(period_length[i]); //convert from the network-endian to the host-endian

    // Receive the length of jp2 image length
    printf("receiving jp2 image length...\n");

    recv_bytes = 0;
    PERIOD_TYPE jp2ImageLen = 0;
    
    while(recv_bytes < sizeof(PERIOD_TYPE) ){

        iResult = recv(ClientSocket, (char*)&jp2ImageLen+recv_bytes, (sizeof(PERIOD_TYPE)-recv_bytes), 0);
        if (iResult > 0) {
            recv_bytes += iResult;
            printf("bytes received: %d\n", recv_bytes);
        }
        else if (iResult == 0){
            printf("Connection closing...\n");
            return;
        }else{
            printf("recv failed with error: %d\n", WSAGetLastError());
            return;
        }
    }
    
    printf("Image length is %d\n", jp2ImageLen);

    printf("receiving jp2 image...\n");

    recv_bytes = 0;
    
    //Set the second output pointer point to the received image
    char* jp2Image = (char*)mxMalloc(jp2ImageLen*sizeof(char));
    
    plhs[1] = mxCreateNumericMatrix(0, 0, mxUINT8_CLASS, mxREAL);
    mxSetData(plhs[1], jp2Image);
    mxSetM(plhs[1], 1);
    mxSetN(plhs[1], jp2ImageLen);

    
    while(recv_bytes < jp2ImageLen) {
        iResult = recv(ClientSocket, jp2Image+recv_bytes, jp2ImageLen, 0);
        if (iResult > 0) {
            //printf("Bytes received: %d\n", iResult);
            recv_bytes += iResult;
        }
        else if (iResult == 0){
            printf("Connection closing...\n");
            return;
        }else{
            printf("recv failed with error: %d\n", WSAGetLastError());
            return;
        }
    }
    printf("received image data %d bytes\n", recv_bytes);

    return;
}
