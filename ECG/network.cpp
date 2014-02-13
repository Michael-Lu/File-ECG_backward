#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include "constants.h"

#include "initsock.h"

CInitSock initSock;

DWORD CECGDlg::WIFIThread(LPVOID lparam)
{
	CECGDlg * pDlg = (CECGDlg *) lparam;
    CString tmp;
	char *buf = new char [BufSize];
	if(buf == NULL)
	{
		pDlg->UpdateStatus(L"In WIFIThread: new buf error!", ADDSTR2STATUS);
        return FALSE;
	}
	DWORD buflen;
#ifdef Debug_PrintSend2NetworkData
	FILE *nn = fopen("network.txt", "w");
#endif
	bool firstGainTransmitted = false;

	DWORD i;

	while(TRUE)
	{
		if(WaitForSingleObject(pDlg->m_ExitWIFIThreadEvent, 0) == WAIT_OBJECT_0)
			break;
		EnterCriticalSection(&m_csCompressedDataBuf);
		if(SharedMem_CompressedDataBufLen == 0)
		{
			LeaveCriticalSection(&m_csCompressedDataBuf);
			continue;
		}
		buflen = SharedMem_CompressedDataBufLen;
		memcpy(buf, SharedMem_CompressedDataBuf, buflen);
#ifdef Debug_PrintSend2NetworkData
		for( i = 0 ; i < buflen ; i++ )
			fprintf(nn, "%d\n", buf[i]);
#endif
		SharedMem_CompressedDataBufLen = 0;
		LeaveCriticalSection(&m_csCompressedDataBuf);

		/*WCHAR strErr[200];
		wsprintf(strErr, L"%d", buflen);
		pDlg->UpdateStatus(strErr, ADDSTR2STATUS);*/

		::send(pDlg->s, buf, buflen, 0);
	}
#ifdef Debug_PrintSend2NetworkData
	fclose(nn);
#endif
	delete [] buf;
	return 0;
}

INT CECGDlg::TCPClientSetup(SOCKET &s, char *url, int port)
{
	SOCKADDR_IN target;
	s = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return 1;

	target.sin_family = AF_INET;
    target.sin_port = htons (port);
    target.sin_addr.S_un.S_addr = inet_addr(url);

	if (::connect(s, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
	{
		WCHAR strErr[200];
		wsprintf(strErr, L"Connect fail code: %d", WSAGetLastError());
		UpdateStatus(strErr, ADDSTR2STATUS);
		return 2;
	}

	return 0;
}

void CECGDlg::OnBnClickedWIFIConnect()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	DWORD IDThread;
	HANDLE hWIFIThread;
	
	//SOCKADDR_IN target;

	UpdateData(TRUE);
	int len = m_strURL.GetLength();
	char *pURL = new char [len];
	if(pURL == NULL)
	{
		UpdateStatus(L"In OnBnClickedWIFIConnect: new pURL error!", ADDSTR2STATUS);
        return;
	}
	for(int i = 0 ; i < len ; i++)
		pURL[i] = (char)m_strURL.GetAt(i);
	pURL[len] = '\0';
	int port = _ttoi(m_strPort);

	int ret;
	ret = TCPClientSetup(s, pURL, port);
	if (ret == 1)
	{
		UpdateStatus(L"Failed to create socket!", ADDSTR2STATUS);
		return;
	}
	else if (ret == 2)
	{
		UpdateStatus(L"Failed to connect to server!", ADDSTR2STATUS);
		::closesocket(s);
		return;
	}

	/*ret = TCPClientSetup(socketGain, pURL, 5566);
	if (ret == 1)
	{
		UpdateStatus(L"Failed to create socket!", ADDSTR2STATUS);
		return;
	}
	else if (ret == 2)
	{
		UpdateStatus(L"Failed to connect to server!", ADDSTR2STATUS);
		::closesocket(socketGain);
		return;
	}*/

	InitializeCriticalSection(&m_csCompressedDataBuf);
	//InitializeCriticalSection(&m_csGainBuf);

	m_ExitWIFIThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	/* Commented by Michael
    hWIFIThread = CreateThread(0, 0, WIFIThread, this, 0, &IDThread);
    if(hWIFIThread == NULL)
    {
		UpdateStatus(L"Failed to create Network threads!", ADDSTR2STATUS);
        return;
    }
    CloseHandle(hWIFIThread);
	*/


	m_BtnWIFIConnect.EnableWindow(FALSE);
	m_BtnWIFIDisconnect.EnableWindow(TRUE);
	m_BtnPhoneConnect.EnableWindow(FALSE);

	//Display status
	UpdateStatus(L"Open Socket!", ADDSTR2STATUS);
	
	delete [] pURL;
}

BOOL CECGDlg::WIFIDisonnect()
{
	::closesocket(s);
	//::closesocket(socketGain);
	return 0;
}

void CECGDlg::OnBnClickedWIFIDisconnect()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	if (m_ExitWIFIThreadEvent != NULL)
	{
		SetEvent(m_ExitWIFIThreadEvent);
		Sleep(1000);
		CloseHandle(m_ExitWIFIThreadEvent);
		m_ExitWIFIThreadEvent = NULL;
	}

	WIFIDisonnect();
	DeleteCriticalSection(&m_csCompressedDataBuf);
	//DeleteCriticalSection(&m_csGainBuf);

	m_BtnWIFIConnect.EnableWindow(TRUE);
	m_BtnWIFIDisconnect.EnableWindow(FALSE);
	m_BtnPhoneConnect.EnableWindow(TRUE);

	//Display status
	UpdateStatus(L"Close Socket!", ADDSTR2STATUS);
}