#ifndef INITSOCK_H
#define INITSOCK_H

#include <winsock.h>

const WORD sockVersion = 0x0202;

class CInitSock
{
public:
	CInitSock()
	{
		WSADATA wsaData;
		if(WSAStartup(sockVersion,&wsaData)!=0)
			exit(0);
		if (wsaData.wVersion != 0x0202)// wrong WinSock version!
		{ 
			::WSACleanup ();
			exit(0);
		}
	}
	~CInitSock() //deconstructor
	{
		::WSACleanup();
	}
};

#endif