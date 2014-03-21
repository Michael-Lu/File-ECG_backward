#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include <bt_api.h>
#include <vector>
#include "NetworkUtil.h"
#include "initsock.h"
#include "constants.h"
#include "QRSDetection.h"
#include "periodEqualization.h"

//My Implementation
#include "ReadFromFile.h"

using namespace std;

CInitSock BTHSock;

vector <BT_ADDR> btaddr_b;

FILE *bth_ECG;
FILE *fp_RIndex;
FILE *fp_periodLength;

//The following constants are used to parse the received bluetooth data,
//in order to get the ECG raw data.
const int RAW_DATA_SIZE = 64;
const BYTE ST_INIT = 0;
const BYTE ST_READ_R = 1;
const BYTE ST_READ_A = 2;
const BYTE ST_READ_W = 3;
const BYTE ST_READ_EQ = 4;
const BYTE ST_READ_6 = 5;
const BYTE ST_READ_4 = 6;
const BYTE ST_ECGDATA_HIGH_BYTE = 7;
const BYTE ST_ECGDATA_LOW_BYTE = 8;
///////////////////////////////////////////////////////////////////////





/*BOOL CECGDlg::OpenPort(LPCTSTR Port, int BaudRate, int DataBits, int Parity, int StopBits, HANDLE &hComm)
{
    
	BOOL ret;
    COMMTIMEOUTS CommTimeOuts;
	m_hComm = CreateFile(Port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if(m_hComm == INVALID_HANDLE_VALUE)
    {
		UpdateStatus(L"Unable to open the serial port or the serial port has already been opened! Please check if the port is already in use", ADDSTR2STATUS);
        //MessageBox(_T("Unable to open the serial port or the serial port has already been opened! Please check if the port is already in use"));
        return FALSE;
    }
	
    dcb.DCBlength = sizeof (dcb);
	ret = GetCommState(m_hComm, &dcb);
	if( !ret)
	{
		UpdateStatus(L"Unable to get Comm. State", ADDSTR2STATUS);
		//MessageBox(_T("Unable to get Comm. State"));
		ClosePort(m_hComm);
		return FALSE;
	}
    dcb.BaudRate = BaudRate;
    dcb.fParity = FALSE;    // Parity check disabled
    dcb.fNull = FALSE;
    dcb.ByteSize = DataBits;
    dcb.Parity = Parity;
    dcb.StopBits = StopBits;

	ret = SetCommState(m_hComm, &dcb);
    if( !ret )
    {
		UpdateStatus(L"Unable to configure serial port. Please check the port configuration! Serial port is closed", ADDSTR2STATUS);
        //MessageBox(_T("Unable to configure serial port. Please check the port configuration! Serial port is closed"));
        ClosePort(m_hComm);
        return FALSE;
    }

    GetCommTimeouts(m_hComm, &CommTimeOuts);
    CommTimeOuts.ReadIntervalTimeout = 100;      // Max text receiving interval
    CommTimeOuts.ReadTotalTimeoutMultiplier = 1;
    CommTimeOuts.ReadTotalTimeoutConstant = 100; // Number of timeouts for reading data
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant = 0;
	ret = SetCommTimeouts(m_hComm, &CommTimeOuts);
    if ( !ret )
    {
		UpdateStatus(L"Unable to set timeout parameter! Serial port is closed!", ADDSTR2STATUS);
        //MessageBox(_T("Unable to set timeout parameter! Serial port is closed!"));
        ClosePort(m_hComm);
        return FALSE;
    }

    PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR); //Discards all characters from the output or input buffer of a specified communications resource.

	return TRUE;
}*/

void CECGDlg::OnBnClickedOpenBTSPP()
{
	// TODO: 在此加入控制項告知處理常式程式碼

    int channel = 0;
    int index = 0;

    PORTEMUPortParams pp;
    memset (&pp, 0, sizeof(pp));
	
	/*CString tmpStr;
	INT device_index = m_ListShowDevice.GetCurSel();
	m_ListShowDevice.GetText(device_index,tmpStr);

	UpdateStatus(L"Connecting to ......", ADDSTR2STATUS);
	UpdateStatus(tmpStr, ADDSTR2STATUS);
	BT_ADDR	b = btaddr_b[device_index];*/

	WCHAR *arg2 = L"000E0006B528";
	WCHAR *arg3 = L"1";
	WCHAR tmp[20];
	swprintf(tmp, L"%d", m_ComboPort.GetCurSel()+4);
	WCHAR *arg4 = tmp;

	//My Implemenation
	/*------------

	if (GetBA(&arg2, &pp.device) && GetDI(&arg3, (unsigned int *)&channel) && GetDI(&arg4, (unsigned int *)&index))
	{
        pp.channel = channel & 0xff;
	}
	else
	{
		UpdateStatus(L"Failed to get device information.", ADDSTR2STATUS);
		return;
	}
	//////////////////////////////////////////////
    //UpdateData(TRUE);
    

	//The zero-based index of the currently selected item in the list box of a combo box, or CB_ERR if no item is selected.
    CString strPort = PortTbl[m_ComboPort.GetCurSel()];
    DWORD dwBaud = BaudTbl[m_ComboBaudRate.GetCurSel()];
    DWORD dwDataBit = DataBitTbl[m_ComboDataBit.GetCurSel()];
    BYTE byParity = ParityTbl[m_ComboParity.GetCurSel()];
    BYTE byStopBit = StopBitTbl[m_ComboStopBit.GetCurSel()];

    BOOL ret = OpenPort(strPort, dwBaud, dwDataBit, byParity, byStopBit, m_hComm);
    if (ret == FALSE)
        return;

	--------------------------*/

	DWORD IDThread;
    HANDLE hBTHRecvThread;
	
	InitializeCriticalSection(&m_csBTHDataBuf);

	m_ExitBTHThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hBTHRecvThread = CreateThread(0, 0, BTHRecvThread, this, 0, &IDThread);
    if(hBTHRecvThread == NULL)
    {
		UpdateStatus(L"Failed to create bluetooth receiving threads!", ADDSTR2STATUS);
        return;
    }
    CloseHandle(hBTHRecvThread);

	m_BtnOpenBTSPP.EnableWindow(FALSE);
    m_BtnCloseBTSPP.EnableWindow(TRUE);
	/*----Commented by Michael---
	m_BtnWIFIDisconnect.EnableWindow(FALSE);
	--------------*/

	//Display status
	UpdateStatus(L"Open Port!", ADDSTR2STATUS);
}

/*******************************************************************************
Function: GetECGRawData
Purpose:  Get the exact ECG data we want
Note:     This is not a flexible code, it depends on the data format we receive.
          For the original data received from BTECG sensor, the data we are interested
		  is 'RAW64='. Once we find 'RAW64=', it should be followed by 65 bytes.
		  The first byte is Carriage Return(decimal: 13), the left 64 bytes are
		  the ECG data, which we should copy these data to specific memory address
		  for further process.
*******************************************************************************/
void CECGDlg::GetECGRawData(char *&buf, double *&data, DWORD bufLen, DWORD &dataLen, BYTE &CurrentState)
{
	//*----Debug
	static FILE *fp_w = NULL;
	static bool flag = true;
	//---*/

	/*-----Debug
	if(flag){
		fp_w = fopen("read_from.txt","w+");
		flag = false;
	}
	
	int orig_size = dataLen;
	/-----*/

	ReadFromFile("death1.txt", data, dataLen);


	/*---Debug----

	for(short i = orig_size; i< dataLen; i++){
			fprintf(fp_w,"%lf ", data[i]);
	}
	/-----------*/

	/*-----Debug
	fclose(fp_w);
	fp_w = NULL;
	------*/

	//My Implementation
	/*--------
	static int RawDataIndex = 0;
	DWORD idx = dataLen;
	static UINT16 ECGSample = 0;
	static bool toPack= false;

	char c;
	for(DWORD i = 0 ; i < bufLen ; i++ )
	{
		//fprintf(m_EcgFp,"%d, %d, %02d, ",CurrentState, PackIndex, RawDataIndex);
		//fprintf(m_EcgFp,"%02X\n", BYTE(buf[i]));
		switch(CurrentState)
		{
			case ST_INIT:
				c = buf[i];
				if( c == 'R' )
					CurrentState = ST_READ_R;
				break;
			case ST_READ_R:
				c = buf[i];
				if( c == 'A' )
					CurrentState = ST_READ_A;
				else
					CurrentState = ST_INIT;
				break;
			case ST_READ_A:
				c = buf[i];
				if( c == 'W' )
					CurrentState = ST_READ_W;
				else
					CurrentState = ST_INIT;
				break;
			case ST_READ_W:	
				c = buf[i];
				if( c == '=' )
					CurrentState = ST_READ_EQ;
				else
					CurrentState = ST_INIT;
				break;
			case ST_READ_EQ:
				c = buf[i];
				if( c == '6' )
					CurrentState = ST_READ_6;
				else
					CurrentState = ST_INIT;
				break;
			case ST_READ_6:
				c = buf[i];
				if( c == '4' )
					CurrentState = ST_READ_4;
				else
					CurrentState = ST_INIT;
				break;
			case ST_READ_4:
				c = buf[i];
				if( c == '\r' )
					CurrentState = ST_ECGDATA_HIGH_BYTE;
				else
					CurrentState = ST_INIT;
				break;
			case ST_ECGDATA_HIGH_BYTE:
				c = buf[i];
				ECGSample = UINT16((BYTE)c)<<8;
				CurrentState = ST_ECGDATA_LOW_BYTE;
				RawDataIndex++;
				break;
			case ST_ECGDATA_LOW_BYTE:
				c = buf[i];
				ECGSample += UINT16(BYTE(c));
				//fprintf(bth_ECG, "%04d\n", ECGSample);
				data[idx++] = double(ECGSample);
				ECGSample = 0;
				RawDataIndex++;
				if(RawDataIndex == RAW_DATA_SIZE)
				{
					RawDataIndex = 0;
					CurrentState = ST_INIT;
				}
				else
					CurrentState = ST_ECGDATA_HIGH_BYTE;
				break;
		}
	}
	dataLen = idx;
	*/
}

DWORD CECGDlg::BTHRecvThread(LPVOID lparam)
{
	CECGDlg * pDlg = (CECGDlg *) lparam;
#ifdef Debug_PrintRIndex
	fp_RIndex = fopen("bthR.txt", "w+");
#endif
#ifdef Debug_PrintECGRawData
	bth_ECG = fopen("Michael_ECG_bth.txt", "w+");
#endif
#ifdef Debug_PrintPeriodLength
	fp_periodLength = fopen("bthA.txt", "w");
#endif

	DWORD dwLength;
	char * recvBuf = new char[BufSize];
	if(recvBuf == NULL)
	{
		pDlg->UpdateStatus(L"In BTHRecvThread: new recvBuf error!", ADDSTR2STATUS);
        return FALSE;
	}
	double *ECGRawData = new double [BufSize*4];
	if(ECGRawData == NULL)
	{
		pDlg->UpdateStatus(L"In BTHRecvThread: new ECGRawData error!", ADDSTR2STATUS);
        return FALSE;
	}
	DWORD ECGRawDataSize = 0;
	DWORD QRSDetectionBeginIndex = 0;
	int receiveTimes = 1;
	bool onSetButNoR = false;

	int RIndexCnt = 0;
	int *RIndex = new int [cPeriodNum];
	if(RIndex == NULL)
	{
		pDlg->UpdateStatus(L"In BTHRecvThread: new RIndex error!", ADDSTR2STATUS);
        return FALSE;
	}
	else
		memset(RIndex, 0, sizeof(int) * cPeriodNum);
	
	int periodCnt = 0;
	int *periodLen = new int [cPeriodNum];
	if(periodLen == NULL)
	{
		pDlg->UpdateStatus(L"In BTHRecvThread: new periodLen error!", ADDSTR2STATUS);
        return FALSE;
	}
	else
		memset(periodLen, 0, sizeof(int) * cPeriodNum);

	double *normalizedData = new double [cNormalizedLen*cNormalizedCnt];
	if(normalizedData == NULL)
	{
		pDlg->UpdateStatus(L"In BTHRecvThread: new normalizedData error!", ADDSTR2STATUS);
        return FALSE;
	}
	else
		memset(normalizedData, 0, sizeof(double) * cPeriodNum*cNormalizedCnt);

	int lastQRSOnsetIndex = 0;
	int prevQRSOnsetIndex = 0;
	double defaultSlope = 0.0;

	//do not write static BYTE CurrentState = 0
	BYTE CurrentState;
	CurrentState = 0;
	WCHAR strErr[255];

	class QRSDetection myQRSDetect;
	class periodEqualization periodEq;
	int normalizedDataTotalLen = 0;
	int restrictLen = BufSize * 3;
	bool skipFirstCycle = true;

	int i, j;
	double slope = 0.0;
	double defaultMaxSlope = 0.0;

	CString dbgstr;



	while(TRUE)
	{
		if(WaitForSingleObject(pDlg->m_ExitBTHThreadEvent, 0) == WAIT_OBJECT_0)
			break;
		/*---------My Implementation
		if(pDlg->m_hComm != INVALID_HANDLE_VALUE)
		{
			BOOL fReadState = ReadFile(pDlg->m_hComm, recvBuf, BufSize, &dwLength, NULL);
			if(!fReadState)
			{
			}
			else
			{
				if(dwLength != 0)
				{

		-------------------------------*/
					//skip the first-time received data
					if( receiveTimes < 5 )
					{
						receiveTimes++;
					}
					else if( receiveTimes >= 5 && receiveTimes <= 12)
					{
						double datum;

						ECGRawDataSize = 0;
						GetECGRawData(recvBuf, ECGRawData, dwLength, ECGRawDataSize, CurrentState);
						/*WCHAR strErr[200];
						wsprintf(strErr, L"%d\n", ECGRawDataSize);
						pDlg->UpdateStatus(strErr, ADDSTR2STATUS);*/
						for( i = 0 ; i < ECGRawDataSize ; i++ )
						{
							datum = myQRSDetect.QRSFilter(ECGRawData[i], false);
							slope = myQRSDetect.calculateSlope(datum, false);
							if( defaultMaxSlope < slope )
							{
								defaultMaxSlope = slope;
							}
						}
						receiveTimes++;
					}
					else if( receiveTimes == 13 )
					{
						ECGRawDataSize = 0;
						GetECGRawData(recvBuf, ECGRawData, dwLength, ECGRawDataSize, CurrentState);
						myQRSDetect.set_defaultMaxSlope(defaultMaxSlope);
						/*WCHAR strErr[200];
						wsprintf(strErr, L"%lf\n", myQRSDetect.get_defaultMaxSlope());
						pDlg->UpdateStatus(strErr, ADDSTR2STATUS);*/

						myQRSDetect.lpfilter(0, true);
						myQRSDetect.hpfilter(0, true);
						myQRSDetect.calculateSlope(0, true);
						receiveTimes++;
					}
					else if( receiveTimes >= 14 && receiveTimes < 18)
					{
						int skipLen;
						if( onSetButNoR == false )
							QRSDetectionBeginIndex = ECGRawDataSize;

						GetECGRawData(recvBuf, ECGRawData, dwLength, ECGRawDataSize, CurrentState);
						myQRSDetect.detectAlg(ECGRawData, QRSDetectionBeginIndex, ECGRawDataSize, RIndex, RIndexCnt, lastQRSOnsetIndex, prevQRSOnsetIndex, onSetButNoR);
						if( RIndexCnt > 0)
						{
							if( RIndex[0] == 0 )
							{
								if(RIndexCnt == 1)
									continue;
								else
									skipLen = RIndex[1];
							}
							else
								skipLen = RIndex[0];

							ECGRawDataSize = ECGRawDataSize - skipLen;
							memmove(ECGRawData, ECGRawData + skipLen, sizeof(double)*ECGRawDataSize);
							
							if( RIndex[0] == 0 )
							{
								for( int i = 0 ; i < (RIndexCnt-1) ; i++ )
									RIndex[i] = RIndex[i+1] - skipLen;

								RIndexCnt--;
							}
							else
							{
								for( int i = 0 ; i < RIndexCnt ; i++ )
									RIndex[i] -= skipLen;
							}
							receiveTimes++;
						}
					}
					else
					{
						//ECGRawDataSize = 0;
						if(ECGRawDataSize > restrictLen)
						{
							CurrentState = ST_INIT;
							ECGRawDataSize = 0;
							onSetButNoR = false;
							RIndexCnt = 0;
							lastQRSOnsetIndex = 0;
							prevQRSOnsetIndex = 0;
							continue;
						}
						if( onSetButNoR == false )
							QRSDetectionBeginIndex = ECGRawDataSize;

						dbgstr.Format(_T("raw_data_start_idx: %d"), count_pre_qrs);
						pDlg->UpdateStatus(dbgstr, ADDSTR2STATUS);

#ifdef Debug_PrintECGRawData
						int raw_startSize = ECGRawDataSize;

#endif


						GetECGRawData(recvBuf, ECGRawData, dwLength, ECGRawDataSize, CurrentState);

#ifdef Debug_PrintECGRawData
						
							for( i = raw_startSize ; i < ECGRawDataSize ; i++ )
								fprintf(bth_ECG, "%04lf\n", ECGRawData[i]);
#endif
						myQRSDetect.detectAlg(ECGRawData, QRSDetectionBeginIndex, ECGRawDataSize, RIndex, RIndexCnt, lastQRSOnsetIndex, prevQRSOnsetIndex, onSetButNoR);


						if( RIndexCnt > 1)
						{
							periodCnt = RIndexCnt - 1;
							for( i = 0 ; i < periodCnt ; i++ )
							{
								periodLen[i] = RIndex[i+1] - RIndex[i];
#ifdef Debug_PrintPeriodLength
								fprintf(fp_periodLength, "%d\n", periodLen[i]);
#endif

#ifdef Debug_PrintRIndex
								fprintf(fp_RIndex, "%d\n", RIndex[i]);
#endif
							}

#ifdef Debug_PrintRIndex
								fprintf(fp_RIndex, "%d\n", RIndex[periodCnt]);
#endif

/*
#ifdef Debug_PrintECGRawData
							for( i = 0 ; i < RIndex[RIndexCnt-1] ; i++ )
								fprintf(bth_ECG, "%04lf\n", ECGRawData[i]);
#endif
*/

							periodEq.periodNormalize(ECGRawData, normalizedData, periodLen, periodCnt, cNormalizedLen, normalizedDataTotalLen);

							EnterCriticalSection(&m_csBTHDataBuf);

							if( (SharedMem_BTHDataBufLen + normalizedDataTotalLen)*sizeof(double) > sizeof(SharedMem_BTHDataBuf) ){
								dbgstr.Format(_T("BTH_SharedMem Overflow!...buflen:%d...newdatalen:%d"), SharedMem_BTHDataBufLen,normalizedDataTotalLen);
								pDlg->UpdateStatus(dbgstr, ADDSTR2STATUS);
								break;
							}else{
								
								//dbgstr.Format( _T("buflen:%d newdatalen:%d"), SharedMem_BTHDataBufLen, normalizedDataTotalLen );
								//pDlg->UpdateStatus(dbgstr, ADDSTR2STATUS);

								memcpy(SharedMem_BTHDataBuf+SharedMem_BTHDataBufLen, normalizedData, sizeof(double) * normalizedDataTotalLen);
								SharedMem_BTHDataBufLen += normalizedDataTotalLen;
							
								memcpy(SharedMem_PeriodLen+SharedMem_PeriodCnt, periodLen, sizeof(int) * periodCnt);
								SharedMem_PeriodCnt += periodCnt;
							}
							
							LeaveCriticalSection(&m_csBTHDataBuf);
						}

						if( RIndexCnt != 0 )
						{
							ECGRawDataSize = ECGRawDataSize - RIndex[RIndexCnt - 1];
							memmove(ECGRawData, ECGRawData + RIndex[RIndexCnt - 1], sizeof(double) * ECGRawDataSize);
							RIndex[0] = 0;
							RIndexCnt = 1;
						}
					}
		/*----My Implementation
				}
			}
		}
		--------*/
	}

	delete [] ECGRawData;
	delete [] recvBuf;
	delete [] RIndex;
	delete [] periodLen;
	delete [] normalizedData;
#ifdef Debug_PrintRIndex
	fclose(fp_RIndex);
#endif
#ifdef Debug_PrintECGRawData
	fclose(bth_ECG);
#endif
#ifdef Debug_PrintPeriodLength
	fclose(fp_periodLength);
#endif

	return 0;
}

/*BOOL CECGDlg::ClosePort(HANDLE &hComm)
{
    if(m_hComm != INVALID_HANDLE_VALUE)
    {
        SetCommMask(m_hComm, 0);
		//Discards all characters from the output or input buffer of a specified communications resource
        PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);
        CloseHandle(m_hComm);
        m_hComm = INVALID_HANDLE_VALUE;
    }

	return TRUE;
}*/

void CECGDlg::OnBnClickedCloseBTSPP()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	if (m_ExitBTHThreadEvent != NULL)
	{
		SetEvent(m_ExitBTHThreadEvent);
		Sleep(1000);
		CloseHandle(m_ExitBTHThreadEvent);
		m_ExitBTHThreadEvent = NULL;
	}

	//My Implementation
	/*----
	ClosePort(m_hComm);
	---*/
	DeleteCriticalSection(&m_csBTHDataBuf);

	m_BtnOpenBTSPP.EnableWindow(TRUE);
	m_BtnCloseBTSPP.EnableWindow(FALSE);

	//Display status
	UpdateStatus(L"Close Port!", ADDSTR2STATUS);
}

void CECGDlg::OnBnClickedSearchDevice()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	Searchdevice();
}

BOOL CECGDlg::Searchdevice()
{
	CString SendStr;

	INT				iResult = 0;
	LPWSAQUERYSET	pwsaResults;
	DWORD			dwSize = 0;
	WSAQUERYSET		wsaq;
	HCURSOR			hCurs;
	HANDLE			hLookup = 0;
	
	UpdateStatus(L"Searching device...", ADDSTR2STATUS);

	//Empty the list box first
	UpdateData();
	m_ListShowDevice.ResetContent();
	UpdateData(FALSE);

	btaddr_b.clear();
	
	//Begin to search bluetooth device
	memset (&wsaq, 0, sizeof(wsaq));
	wsaq.dwSize      = sizeof(wsaq);
	wsaq.dwNameSpace = NS_BTH;
	wsaq.lpcsaBuffer = NULL;

	iResult = WSALookupServiceBegin(&wsaq, LUP_CONTAINERS, &hLookup);
	if (ERROR_SUCCESS != iResult)
	{
		TCHAR tszErr[32];
		iResult = WSAGetLastError();
		StringCchPrintf(tszErr, 32, _T("Socket Error: %d."), iResult);
		UpdateStatus(tszErr, ADDSTR2STATUS);
		WSALookupServiceEnd(hLookup);
		return FALSE;
	}

	union {
		CHAR buf[5000];				// returned struct can be quite large 
		SOCKADDR_BTH	__unused;	// properly align buffer to BT_ADDR requirements
	};

	// save the current cursor
	hCurs = GetCursor();
	// set the wait cursor while searching
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	pwsaResults = (LPWSAQUERYSET) buf;
	dwSize  = sizeof(buf);
	memset(pwsaResults, 0, sizeof(WSAQUERYSET));
	pwsaResults->dwSize = sizeof(WSAQUERYSET);
	pwsaResults->dwNameSpace = NS_BTH;
	pwsaResults->lpBlob = NULL;

	while(1)
	{
		iResult = WSALookupServiceNext(hLookup,
										LUP_RETURN_NAME | LUP_RETURN_ADDR,
										&dwSize,
										pwsaResults);
		if(ERROR_SUCCESS != iResult)
		{
			iResult = WSAGetLastError();
			//if the error is occur NOT because there is no more device
			if (WSA_E_NO_MORE != iResult)
			{
				TCHAR tszErr[32];
				iResult = WSAGetLastError();
				StringCchPrintf(tszErr, 32, _T("Socket Error: %d."), iResult);
				UpdateStatus(tszErr, ADDSTR2STATUS);
				WSALookupServiceEnd(hLookup);
				SetCursor(hCurs);
				return FALSE;
			}
			//finished
			break;
		}
		if(pwsaResults->lpszServiceInstanceName)
		{
			UpdateData();
			m_ListShowDevice.InsertString(m_ListShowDevice.GetCount(), (LPCTSTR)pwsaResults->lpszServiceInstanceName);
			UpdateData(FALSE);
			btaddr_b.push_back(((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr);
		}
	}
	WSALookupServiceEnd(hLookup);
	SetCursor(hCurs);

	UpdateStatus(L"Searching device completed.", ADDSTR2STATUS);

	return TRUE;
}

