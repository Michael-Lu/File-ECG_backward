#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include <cstdio>
#include <stddef.h>
#include <cstdlib>
#include <wchar.h>
#include <bt_api.h>
#include <ras.h>
#include <raserror.h>
#include "NetworkUtil.h"
#include "initsock.h"
#include "constants.h"

DWORD CECGDlg::PhoneThread(LPVOID lparam)
{
	CECGDlg * pDlg = (CECGDlg *) lparam;
    CString tmp;
	DWORD dwLength2;
	char *buf = new char [BufSize];
	if(buf == NULL)
	{
		pDlg->UpdateStatus(L"InPhoneThread: new buf error!", ADDSTR2STATUS);
        return FALSE;
	}

#ifdef Debug_PrintSend2PhoneData
	FILE *nn = fopen("phone.txt", "w");
#endif

	while(TRUE)
	{
		if(WaitForSingleObject(pDlg->m_ExitPhoneThreadEvent, 0) == WAIT_OBJECT_0)
			break;
		
		BOOL fReadState = ReadFile(pDlg->m_hComm2, buf, BufSize, &dwLength2, NULL);
		if(!fReadState)
		{
		}
		else
		{
			if(dwLength2 != 0)
			{
#ifdef Debug_PrintShapeCodebook
				fprintf(nn, "%s\n", buf);
#endif
			}
		}
		
	}
#ifdef Debug_PrintShapeCodebook
	fclose(nn);
#endif
	delete [] buf;
	return 0;
}


void CECGDlg::OnBnClickedPhoneConnect()
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

	//WCHAR *arg2 = L"3C5A37A8F2A5"; //This is for Weing Tsai's phone
	WCHAR *arg2 = L"D0DFC78CA710"; //This is for Michael Lu's phone
	//WCHAR *arg2 = L"00106056EF8A";
	WCHAR *arg3 = L"1";
	WCHAR tmp[20];
	swprintf(tmp, L"%d", m_ComboPort.GetCurSel()+5);
	WCHAR *arg4 = tmp;

	if (GetBA(&arg2, &pp.device) && GetDI(&arg3, (unsigned int *)&channel) && GetDI(&arg4, (unsigned int *)&index))
	{
        pp.channel = channel & 0xff;
		pp.uiportflags = RFCOMM_PORT_FLAGS_REMOTE_DCB;
	}
	/*if (GetDI(&arg3, (unsigned int *)&channel) && GetDI(&arg4, (unsigned int *)&index))
	{
		pp.device = b;
        pp.channel = channel & 0xff;
	}*/
	else
	{
		UpdateStatus(L"Failed to get device information.", ADDSTR2STATUS);
		return;
	}
	
    //UpdateData(TRUE);

	DWORD IDThread;
    HANDLE hPhoneThread;

	//The zero-based index of the currently selected item in the list box of a combo box, or CB_ERR if no item is selected.
    CString strPort = PortTbl[m_ComboPort.GetCurSel()+1];
    DWORD dwBaud = BaudTbl[m_ComboBaudRate.GetCurSel()];
    DWORD dwDataBit = DataBitTbl[m_ComboDataBit.GetCurSel()];
    BYTE byParity = ParityTbl[m_ComboParity.GetCurSel()];
    BYTE byStopBit = StopBitTbl[m_ComboStopBit.GetCurSel()];

    /*BOOL ret = OpenPort(strPort, dwBaud, dwDataBit, byParity, byStopBit, m_hComm2);
    if (ret == FALSE)
        return;*/
//////////////////////////////////////////////
	WCHAR szComPort[30]={0};
	wsprintf (szComPort, L"COM%d:", index);
	HKEY hk;
	if(ERROR_SUCCESS!=RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"ExtModems\\Bluetooth DUN", 0, NULL, 0,KEY_ALL_ACCESS, NULL,&hk,NULL))
	{
		UpdateStatus(L"Bluetooth ExtModems\\BluetoothDUN OPEN fail!", ADDSTR2STATUS);
		return;        
	}
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"ExtModems\\Bluetooth DUN", 0, KEY_READ, &hk))
	{
		UpdateStatus(L"Bluetooth ExtModems\\BluetoothDUN OPEN fail!", ADDSTR2STATUS);
		return;        
	}
	
	DWORD dwSize = sizeof(szComPort);
	if(ERROR_SUCCESS != RegSetValueEx(hk, L"Port", 0, REG_SZ, (LPBYTE)szComPort, dwSize))
	{
		UpdateStatus(L"set Bluetooth 's modem port fail!", ADDSTR2STATUS);
		RegCloseKey (hk);            
		return;
	}
	
	DWORD dwDevicetype=1;
	if(ERROR_SUCCESS != RegSetValueEx(hk,L"DeviceType", 0, REG_DWORD, (LPBYTE)&dwDevicetype, sizeof(DWORD)))
	{
		UpdateStatus(L"set Bluetooth 's modem devicetype fail!", ADDSTR2STATUS);
		RegCloseKey (hk);            
		return;
	}
	if(ERROR_SUCCESS != RegSetValueEx(hk,L"FriendlyName", 0, REG_SZ,(LPBYTE)L"BluetoothDUN", sizeof(L"BluetoothDUN")))
	{
		UpdateStatus(L"set Bluetooth 's modem FriendlyName fail!", ADDSTR2STATUS);
		RegCloseKey (hk);            
		return;
	}

	HRASCONN hRASConnection = NULL;
    DWORD nRet;

    RASENTRYNAME TempEntryName;
    TempEntryName.dwSize = sizeof(RASENTRYNAME);
    unsigned long EntryBufferSize = sizeof(TempEntryName);
    unsigned long EntryWritten = 0;

    //ERROR_BUFFER_TOO_SMALL is expected
    nRet = RasEnumEntries( NULL, NULL, &TempEntryName, &EntryBufferSize, &EntryWritten );
    if (nRet != ERROR_BUFFER_TOO_SMALL)
    {
        WCHAR strErr[255];
        wsprintf(strErr,L"RasEnumEntries failed: Error %d\n", nRet);
        UpdateStatus(L"GPRS Monitor", ADDSTR2STATUS);
        return;
    }

    RASENTRYNAME *RasEntryNameArray = (RASENTRYNAME*)malloc(EntryBufferSize);
    RasEntryNameArray[0].dwSize = sizeof(RASENTRYNAME);
    nRet = RasEnumEntries( NULL, NULL, RasEntryNameArray, &EntryBufferSize, &EntryWritten );
    if (0 != nRet)
    {
        WCHAR strErr[255];
        wsprintf(strErr,L"RasEnumEntries failed: Error %d\n", nRet);
        UpdateStatus(L"GPRS Monitor", ADDSTR2STATUS);
        return;
    }

    int GPRSEntry = -1;

    RASENTRY RasEntry;
    memset( &RasEntry, 0, sizeof(RasEntry) );
    RasEntry.dwSize = sizeof(RasEntry);
    DWORD dwEntrySize = sizeof(RasEntry);
    unsigned char Buffer[4098];
    memset( Buffer, 0, sizeof(Buffer) );
    DWORD dwBufferSize = sizeof(Buffer);

    for ( unsigned long iEntry = 0; iEntry < EntryWritten; iEntry++ )
    {
        // Check if the name has GPRS in it 
        // AND
        // if Local Phone Number contains "~GPRS!"
        nRet = RasGetEntryProperties(NULL, RasEntryNameArray[iEntry].szEntryName, &RasEntry,&dwEntrySize,NULL,NULL);
        if (0 != nRet)
        {
            WCHAR strErr[255];
            wsprintf(strErr,L"RasGetEntryProperties failed: Error %d\n", nRet);
            UpdateStatus(strErr, ADDSTR2STATUS);
            continue;
        }

		if ((wcsstr(RasEntry.szLocalPhoneNumber,L"*99#") != NULL) 
            &&
            (wcsstr(RasEntryNameArray[iEntry].szEntryName, L"phone") != NULL))
        {    
            //RAS entry is GPRS - exit 'for' loop
            GPRSEntry = iEntry;
            break;
        }
    }

    nRet = RasGetEntryProperties( NULL, RasEntryNameArray[GPRSEntry].szEntryName, &RasEntry, &dwEntrySize, Buffer, &dwBufferSize );
    if(0 != nRet)
    {
        WCHAR strErr[255];
        wsprintf(strErr,L"RasGetEntryProperties failed: Error %d\n", nRet);
        UpdateStatus(strErr, ADDSTR2STATUS);
        return;
    }

    // Configure the RASDIALPARAMS structure
    RASDIALPARAMS RASDialParameters;
    memset( &RASDialParameters,0,sizeof( RASDIALPARAMS ) );
    RASDialParameters.szPhoneNumber[0] = NULL; //TEXT('\0');
    RASDialParameters.szCallbackNumber[0] = NULL; //TEXT('\0');
    RASDialParameters.dwSize = sizeof( RASDIALPARAMS );
    wcscpy( RASDialParameters.szEntryName, RasEntryNameArray[GPRSEntry].szEntryName);
    RASDialParameters.szUserName[0] = TEXT('\0');
    RASDialParameters.szPassword[0] = TEXT('\0');
    RASDialParameters.szDomain[0] = TEXT('\0');
    //wcscpy (RasDialParams.szUserName, szUserName); //This is optional    
    //wcscpy (RasDialParams.szPassword, szPassword); //This is optional
    //wcscpy (RasDialParams.szDomain, szDomain); //This is optional


    //try reuse GPRS connection
    BOOL bPassword = FALSE;
    nRet = RasGetEntryDialParams(NULL, &RASDialParameters, &bPassword);
    if (0 != nRet)
    {
        WCHAR strErr[255];
        wsprintf(strErr,L"RasGetEntryDialParams failed: Error %d\n", nRet);
        UpdateStatus(strErr, ADDSTR2STATUS);
        return;
    }

    //free resources not on the stack
    free ((VOID*)RasEntryNameArray);
    
                
    //  Try to establish RAS connection.
	nRet = RasDial( NULL, NULL, &RASDialParameters, 
            NULL, // Notifier type is NOT a window handle
            NULL, // Window receives notification message - none
            &hRASConnection );
    if ( nRet != 0 )
    {
        WCHAR strErr[255];
		wsprintf(strErr,L"RasDial failed: Error %d\n", nRet);
        UpdateStatus(strErr, ADDSTR2STATUS);
        if ( hRASConnection != NULL )
            RasHangUp( hRASConnection );
        hRASConnection = NULL;
        UpdateStatus(L"Could not connect using RAS", ADDSTR2STATUS);
        return;
    }

    hRASConnection = NULL;

	//////////////////////////////////////////////
	//InitializeCriticalSection(&m_csCompressedDataBuf);
	//InitializeCriticalSection(&m_csGainBuf);

	m_ExitPhoneThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hPhoneThread = CreateThread(0, 0, PhoneThread, this, 0, &IDThread);
    if(hPhoneThread == NULL)
    {
		UpdateStatus(L"Failed to create Network threads!", ADDSTR2STATUS);
        return;
    }
    CloseHandle(hPhoneThread);

	m_BtnPhoneConnect.EnableWindow(FALSE);
	m_BtnPhoneDisconnect.EnableWindow(TRUE);

	//Display status
	UpdateStatus(L"Phone Connect!", ADDSTR2STATUS);
}

void CECGDlg::OnBnClickedPhoneDisconnect()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	if (m_ExitPhoneThreadEvent != NULL)
	{
		SetEvent(m_ExitPhoneThreadEvent);
		Sleep(1000);
		CloseHandle(m_ExitPhoneThreadEvent);
		m_ExitPhoneThreadEvent = NULL;
	}

	ClosePort(m_hComm2);
	DeleteCriticalSection(&m_csCompressedDataBuf);
	DeleteCriticalSection(&m_csGainBuf);

	m_BtnWIFIConnect.EnableWindow(TRUE);
	m_BtnPhoneConnect.EnableWindow(TRUE);
	m_BtnPhoneDisconnect.EnableWindow(FALSE);

	//Display status
	UpdateStatus(L"Close Phone Connection!", ADDSTR2STATUS);
}

