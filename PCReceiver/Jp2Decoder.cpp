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
#include <string.h>


/* For Jasper */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <jasper/jasper.h>
/* ----*/

#pragma comment (lib, "Ws2_32.lib")

/* Some parameters which must be coherent with the sender */
#define PERIOD_TYPE unsigned long //should never be another type!!
#define PERIOD_NUM 10
/*------------*/

static SOCKET wait_con_on_socket(char* addr, char* port);
static int recv_data(const SOCKET ClientSocket,PERIOD_TYPE *const p_period, char* *const pp_jp2_image, PERIOD_TYPE *const p_jp2_size);
static int decode_jp2(char *const jp2_image, const int jp2_size, unsigned char* *const raw_image, int *const pM, int *const pN);

/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{


    WSADATA wsaData;
	int iResult;

	
	if(nrhs != 1){
		mexPrintf("There should be exactly 1 input arguments, 'mode = init(0) || receive(1) || close(2)'\n");
		return;
	}
	
	if( ! mxIsClass(prhs[0], "char") ){
		mexPrintf("The input argument should be class of char\n");
		return;
	}
	
	char* mode_str = mxArrayToString(prhs[0]);
	enum {INIT, RECEIVE, CLOSE, UNDEFINED} mode;
	if( strcmp(mode_str, "init") == 0){
		mode = INIT;
	}else if( strcmp(mode_str, "receive") == 0){
		mode = RECEIVE;
	}else if( strcmp(mode_str, "close") == 0){
		mode = CLOSE;
	}else{
		mode = UNDEFINED;
	}

	
	static int recv_socket;
	
	
	unsigned char *raw_data = NULL; //Note that Matlab follows column-major paradigm	
	unsigned long *period;
	long int *err;
	
	int M,N; //dimension of the raw_data matrix
	FILE* f_jp2;
	PERIOD_TYPE size;
	char *buf;
	
	
	switch(mode){
	case INIT:
		printf("INIT\n");
		
		if(nlhs != 1){
			mexPrintf("There should be exactly 1 output arguments, [err]\n");
			return;
		}
		plhs[0] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL); //for err
		err = (long int*) mxGetData(plhs[0]);
		*err = 0;
		
	    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			*err =  iResult;
			return;
		}
		
		recv_socket = wait_con_on_socket("140.113.13.53", "4567");
		if(  recv_socket == INVALID_SOCKET){
			*err = errno;
			return;
		}
		
		if (jas_init()) {
			printf("jas_init() failed!\n");
			*err = -1;
			return;
		}
		
		
		break;
		
	case RECEIVE:
		printf("RECEIVE\n");
		
		
		if(nlhs != 3){
			mexPrintf("There should be exactly 3 output arguments, [raw_data, period, err]\n");
			return;
		}
	
		plhs[0] = mxCreateNumericMatrix(0, 0, mxUINT8_CLASS, mxREAL); // for raw_data
		plhs[1] = mxCreateNumericMatrix(PERIOD_NUM, 1, mxUINT32_CLASS, mxREAL); //for preiod
		plhs[2] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL); //for err
	
		period = (unsigned long*)mxGetData(plhs[1]);
		err = (long int*) mxGetData(plhs[2]);
		*err = 0;
		
	/*	----For Debugging decode_jp2()--------
		f_jp2 = fopen("Sunset.jp2", "rb");
		fseek (f_jp2, 0, SEEK_END);
		size = ftell(f_jp2);
		fseek(f_jp2, 0, SEEK_SET);
		
		buf = (char*)malloc(size);
		
		if( size != fread(buf, 1, size, f_jp2) ){
			printf("error in fread\n");
		}
		fclose(f_jp2);
	----------------------------------------------*/
	
		if( iResult = recv_data(recv_socket, period, &buf, &size) == 0){
			printf("Tranferring Done!!\n"); //Will continue to execute case:Close
			goto DONE;
		}
		if( iResult == -1){
			*err = iResult;
			break;
		}
		
		if(decode_jp2(buf, size, &raw_data, &M, &N) ){
			printf("error in decode_jp2\n");
			break;
		}
		
		mxSetData(plhs[0], raw_data);
		mxSetM(plhs[0], M);
		mxSetN(plhs[0], N);	
	/* ----For Dubugging decode_jp2()-----
		free(buf);
	------------------------------------*/
		break;
		
		
	case CLOSE:
		printf("CLOSE\n");
		fflush(stdout);
		
		if(nlhs != 1){
			mexPrintf("There should be exactly 1 output arguments, [err]\n");
			return;
		}
		plhs[0] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL); //for err
		err = (long int*) mxGetData(plhs[0]);
		*err = 0;

DONE:
		jas_image_clearfmts(); //clean-up for the jas_init()
		
		shutdown(recv_socket, SD_BOTH);
		closesocket(recv_socket);
		if( WSACleanup() != 0){
			printf("WSACleanup() failed with %d\n", WSAGetLastError());
			*err = WSAGetLastError();
		}
		break;
		
	default:
		printf("unrecognize the input argument, %s\n", mode_str);
		break;
	
	}
	mxFree(mode_str);


    return;
}


static SOCKET wait_con_on_socket(char* addr, char* port = "4567"){
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
	printf("Listening on port %s\n", port);
    printf("Listening on socket %d\n", ListenSocket);
	fflush(stdout);

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

static int recv_data(const SOCKET ClientSocket,PERIOD_TYPE *const p_period, char* *const pp_jp2_image, PERIOD_TYPE *const p_jp2_size){
	
	printf("receiving period lengths...\n");

	int iResult;
	int recv_bytes = 0; //count the number of received bytes when receiving period length
	
	char *recvbuf = (char*)p_period;
	ZeroMemory(recvbuf, PERIOD_NUM * sizeof(PERIOD_TYPE) );
	while(recv_bytes < PERIOD_NUM * sizeof(PERIOD_TYPE)){

		iResult = recv(ClientSocket, recvbuf+recv_bytes, (PERIOD_NUM * sizeof(PERIOD_TYPE) - recv_bytes), 0);
		if (iResult > 0) {
			recv_bytes += iResult;
			//printf("bytes received: %d\n", recv_bytes);
		}
		else if (iResult == 0){
			printf("Connection closing...\n");
			return 0;
		}else{
			printf("recv periods failed with error: %d\n", WSAGetLastError());
			return -1;
		}
	}

	for(short i=0; i<PERIOD_NUM ; i++)
		p_period[i] = ntohl(p_period[i]); //write the received periods to the file

	// Receive the length of jp2 image length
	printf("receiving jp2 image length...\n");

	recvbuf = (char*)p_jp2_size;
	ZeroMemory(recvbuf, sizeof(PERIOD_TYPE));
	recv_bytes = 0;
	while(recv_bytes < sizeof(PERIOD_TYPE) ){

		iResult = recv(ClientSocket, recvbuf+recv_bytes, (sizeof(PERIOD_TYPE)-recv_bytes), 0);
		if (iResult > 0) {
			recv_bytes += iResult;
			//printf("bytes received: %d\n", recv_bytes);
		}
		else if (iResult == 0){
			printf("Connection closing...\n");
			return 0;
		}else{
			printf("recv jp2_size failed with error: %d\n", WSAGetLastError());
			return -1;
		}
	}

	*p_jp2_size = *p_jp2_size; //should be: ntohl(*p_jp2_size), but the WinCE sender haven't add htonl();
	printf("Image length is %d\n", *p_jp2_size);

	printf("receiving jp2 image...\n");
	
	*pp_jp2_image = (char*)mxMalloc( sizeof(unsigned char) * (*p_jp2_size) );
	
	recvbuf = *pp_jp2_image;
	int jp2_size = *p_jp2_size;
	ZeroMemory(recvbuf, jp2_size);
	recv_bytes = 0;
	while(recv_bytes < jp2_size) {

			
			iResult = recv(ClientSocket, recvbuf+recv_bytes, (jp2_size-recv_bytes), 0);
			if (iResult > 0) {
				//printf("Bytes received: %d\n", iResult);
				recv_bytes += iResult;
			}
			else if (iResult == 0){
				printf("Connection closing...\n");
				return 0;
			}else{
				printf("recv failed with error: %d\n", WSAGetLastError());
				return -1;
			}
	}
	printf("received image data %d bytes\n", recv_bytes);
	
	return recv_bytes;
}

static int decode_jp2(char *const jp2_image, const int jp2_size, unsigned char* *const raw_image, int *const pM, int *const pN){
//Before executing this function, jas_init() should be called!


//If success, return 0;otherwise, return -1;
//char* raw_image points to the start of buffer allocated to store the decoded jp2 image;
	
	jas_image_t *image;
	jas_stream_t *in = jas_stream_memopen(jp2_image, jp2_size); //let the stream allocate the growable memory spaces automatically
	if(!in){
		printf("Error in decode_jp2(): Cannot create jas_stream, in!");
		return -1;
	}

	if (!(image = jas_image_decode(in, 4, NULL))) {
		printf("Error in decode_jp2(): cannot decode image data\n");
		return -1;
	}
	
	jas_stream_close(in);
	
	jas_stream_flush(image->cmpts_[0]->stream_);
	jas_stream_memobj_t *obj = (jas_stream_memobj_t*)image->cmpts_[0]->stream_->obj_;
	int width = image->cmpts_[0]->width_;
	int height = image->cmpts_[0]->height_;
	*pM = height;
	*pN = width;
	
	assert(obj->len_ == width*height);
	
	unsigned char *temp = (unsigned char*)mxCalloc(obj->len_, sizeof(unsigned char)); //obj->len is the lenghth of the data started at obj->buf
	*raw_image = (unsigned char*)mxCalloc(obj->len_, sizeof(unsigned char)); //Allocate the memory for the return argument, raw_data
	
	//Note that if mxMalloc failed, program will immediately return to Matlab prompt, and memory leaks occur since
	//jas_image_destroy(image) won't be executed.
	memcpy(temp, obj->buf_, obj->len_);
	
	jas_image_destroy(image);

	
	//The following block transforms the memory map from row-major to col-major
	for(short i = 0 ; i < height ; i++){
		for(short j = 0 ; j < width ; j++){
			(*raw_image)[i + j*height] = temp[i*width + j];
		}
	}
	mxFree(temp);

	
	return 0;
}

