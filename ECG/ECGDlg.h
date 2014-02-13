// ECGDlg.h : header file
//
#ifndef ECGDLG_H
#define ECGDLG_H

//#pragma once

#include <stdio.h>
#include "afxwin.h"
#include "VQClass.h"
#include "CodeBook.h"

const int BufSize = 512;
const int PacketSize = 128;
const int SIZEofCHAR = 8;

const BOOL ADDSTR2STATUS = FALSE;
const BOOL CLEARSTATUS = TRUE;

// CECGDlg dialog
class CECGDlg : public CDialog
{
// Construction
public:
	CECGDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ECG_DIALOG };


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()

private:
	SOCKET s;
	SOCKET socketGain;
	/***************************GUI component***************************/
	CListBox m_ListShowDevice;
	CComboBox m_ComboPort;
	CComboBox m_ComboBaudRate;
	CComboBox m_ComboDataBit;
	CComboBox m_ComboParity;
	CComboBox m_ComboStopBit;

	CButton m_BtnSearchDevice;
	CButton m_BtnOpenBTSPP;
	CButton m_BtnCloseBTSPP;
	CButton m_BtnWIFIConnect;
	CButton m_BtnWIFIDisconnect;
	CButton m_BtnPhoneConnect;
	CButton m_BtnPhoneDisconnect;
	CButton m_BtnStart;
	CButton m_BtnEnd;
	CButton m_BtnReset;
	CButton m_BtnClear;

	CString m_strURL;
	CString m_strPort;
	CString m_strCodeBookFilename;
	CString m_strSamplePoint;
	CString m_strDimension;
	CString m_strGainCodeBookFilename;
	CString m_strGainSamplePoint;
	CString m_strGainDimension;
	CString m_strY;

	CString m_strStatus;
	
	/************************bluetooth IO handler************************/
	DCB dcb;
	HANDLE m_hComm;
	HANDLE m_hComm2;

	/****************************thread event****************************/
	HANDLE m_ExitBTHThreadEvent;
	HANDLE m_ExitVQThreadEvent;
	HANDLE m_ExitWIFIThreadEvent;
	HANDLE m_ExitPhoneThreadEvent;
	static CRITICAL_SECTION m_csBTHDataBuf;
	static CRITICAL_SECTION m_csCompressedDataBuf;
	static CRITICAL_SECTION m_csGainBuf;

	/****************************shared memory***************************/
	//static變數不可以在constructor做初始化
	static double SharedMem_BTHDataBuf[cBTH_Shared_Mem_Size_PeriodCnt*cNormalizedLen];  //originally is BufSize*4
	static DWORD SharedMem_BTHDataBufLen;
	static char SharedMem_CompressedDataBuf[BufSize];
	static DWORD SharedMem_CompressedDataBufLen;
	static int SharedMem_PeriodLen[cBTH_Shared_Mem_Size_PeriodCnt];  //originally is cPeriodNum
	static DWORD SharedMem_PeriodCnt;
	//static double SharedMem_Gain;
	//static bool SharedMem_GainReady;

	BOOL Searchdevice();
	BOOL OpenPort(LPCTSTR Port, int BaudRate, int DataBits, int Parity, int StopBits, HANDLE &hComm);
	BOOL ClosePort(HANDLE &hComm);
	BOOL WIFIDisonnect();
	INT TCPClientSetup(SOCKET &, char *, int );
	BOOL End();

	BOOL LoadCodeBook();
	BOOL LoadGainCodeBook();

	BOOL SendStartCommand();
	BOOL SendEndCommand();


	CTime t1, t2;
	
	BOOL CreateCodeWord(double **&, INT, INT);
	void DeleteCodeWord(double **&, INT );
public:

	/* Note! Originally UpdateStatus is declared as private member function of class CECGDlg
	   Howeever, for debugging purpose, I make it as a public member function.
	   Written by Michael Lu 2013/09/09
	*/
	void UpdateStatus(CString, BOOL);


	static void GetECGRawData(char *&, double *&, DWORD, DWORD &, BYTE &);
	static bool pack(int &tx_data, int &idx, int &bits_cnt);

	static INT countM(int d)
	{
		INT i = 0;
		while(d != 1)
		{
			d >>= 1; //equivalent to "d /= 2"
			i++;
		}
		return i;
	}

	static DWORD BTHRecvThread(LPVOID lparam);
	static DWORD VQAlgThread(LPVOID lparam);
	static DWORD WIFIThread(LPVOID lparam);
	static DWORD PhoneThread(LPVOID lparam);

	afx_msg void OnBnClickedSearchDevice();
	afx_msg void OnBnClickedOpenBTSPP();
	afx_msg void OnBnClickedCloseBTSPP();
	afx_msg void OnBnClickedWIFIConnect();
	afx_msg void OnBnClickedWIFIDisconnect();
	afx_msg void OnBnClickedPhoneConnect();
	afx_msg void OnBnClickedPhoneDisconnect();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedEnd();
	
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedReset();
};


#endif