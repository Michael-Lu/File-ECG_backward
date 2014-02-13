/*
File name: Comm.cpp
Purpose  : This implementation includes two function. One is open port, and the
		   other is close port.
*/
#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

BOOL CECGDlg::OpenPort(LPCTSTR Port, int BaudRate, int DataBits, int Parity, int StopBits, HANDLE &hComm)
{
    DCB dcb;
	BOOL ret;
    COMMTIMEOUTS CommTimeOuts;
	hComm = CreateFile(Port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if(hComm == INVALID_HANDLE_VALUE)
    {
		UpdateStatus(L"Unable to open the serial port or the serial port has already been opened! Please check if the port is already in use", ADDSTR2STATUS);
        return FALSE;
    }
	
    dcb.DCBlength = sizeof (dcb);
	ret = GetCommState(hComm, &dcb);
	if( !ret)
	{
		UpdateStatus(L"Unable to get Comm. State", ADDSTR2STATUS);
		ClosePort(hComm);
		return FALSE;
	}
    dcb.BaudRate = BaudRate;
    dcb.fParity = FALSE;    // Parity check disabled
    dcb.fNull = FALSE;
    dcb.ByteSize = DataBits;
    dcb.Parity = Parity;
    dcb.StopBits = StopBits;

	ret = SetCommState(hComm, &dcb);
    if( !ret )
    {
		UpdateStatus(L"Unable to configure serial port. Please check the port configuration! Serial port is closed", ADDSTR2STATUS);
        ClosePort(hComm);
        return FALSE;
    }

    GetCommTimeouts(hComm, &CommTimeOuts);
    CommTimeOuts.ReadIntervalTimeout = 100;      // Max text receiving interval
    CommTimeOuts.ReadTotalTimeoutMultiplier = 1;
    CommTimeOuts.ReadTotalTimeoutConstant = 100; // Number of timeouts for reading data
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant = 0;
	ret = SetCommTimeouts(hComm, &CommTimeOuts);
    if ( !ret )
    {
		UpdateStatus(L"Unable to set timeout parameter! Serial port is closed!", ADDSTR2STATUS);
        ClosePort(hComm);
        return FALSE;
    }

    PurgeComm(hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);

	return TRUE;
}

BOOL CECGDlg::ClosePort(HANDLE &hComm)
{
    if(hComm != INVALID_HANDLE_VALUE)
    {
        SetCommMask(hComm, 0);
        PurgeComm(hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);
        CloseHandle(hComm);
        hComm = INVALID_HANDLE_VALUE;
    }

	return TRUE;
}