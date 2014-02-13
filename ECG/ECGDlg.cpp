// ECGDlg.cpp : implementation file
//
///////////////////////////////////////
//This for test
///////////////////////////////////////
#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include "CodeBook.h"
#include "gainCodebook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

double CECGDlg::SharedMem_BTHDataBuf[cBTH_Shared_Mem_Size_PeriodCnt*cNormalizedLen];  //This received ECG data has been period-normalized!!
DWORD CECGDlg::SharedMem_BTHDataBufLen = 0;
char CECGDlg::SharedMem_CompressedDataBuf[BufSize];
DWORD CECGDlg::SharedMem_CompressedDataBufLen = 0;
int CECGDlg::SharedMem_PeriodLen[cBTH_Shared_Mem_Size_PeriodCnt];
DWORD CECGDlg::SharedMem_PeriodCnt;
//double CECGDlg::SharedMem_Gain;
//bool CECGDlg::SharedMem_GainReady = false;

VQCodeBook vqcb;
gainCodebook gcb;

CRITICAL_SECTION CECGDlg::m_csBTHDataBuf;
CRITICAL_SECTION CECGDlg::m_csCompressedDataBuf;
CRITICAL_SECTION CECGDlg::m_csGainBuf;


// CECGDlg dialog

CECGDlg::CECGDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CECGDlg::IDD, pParent)
	, m_strURL(_T("140.113.13.53"))
	, m_strPort(_T("4567"))
	, m_strSamplePoint(_T("4"))
	, m_strDimension(_T("64"))
	, m_strCodeBookFilename(_T("codebook.txt"))
	, m_strGainSamplePoint(_T("36"))
	, m_strGainDimension(_T("64"))
	, m_strGainCodeBookFilename(_T("gaincodebook.txt"))
	, m_strStatus(_T(""))
	, m_strY(_T("Y.txt"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CECGDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_BTHPORT, m_ComboPort);
	DDX_Control(pDX, IDC_COMBO_BTHBAUDRATE, m_ComboBaudRate);
	DDX_Control(pDX, IDC_COMBO_BTHDATABIT, m_ComboDataBit);
	DDX_Control(pDX, IDC_COMBO_BTHPARITY, m_ComboParity);
	DDX_Control(pDX, IDC_COMBO_BTHSTOPBIT, m_ComboStopBit);
	DDX_Control(pDX, IDC_BTN_OpenBTSPP, m_BtnOpenBTSPP);
	DDX_Control(pDX, IDC_BTN_CloseBTSPP, m_BtnCloseBTSPP);


	DDX_Control(pDX, IDC_BTN_WIFICONNECT, m_BtnWIFIConnect);
	DDX_Control(pDX, IDC_BTN_WIFIDISCONNECT, m_BtnWIFIDisconnect);
	DDX_Control(pDX, IDC_BTN_PHONECONNECT, m_BtnPhoneConnect);
	DDX_Control(pDX, IDC_BTN_PHONEDISCONNECT, m_BtnPhoneDisconnect);
	DDX_Text(pDX, IDC_EDIT_URL, m_strURL);
	DDX_Text(pDX, IDC_EDIT_PORT, m_strPort);
	DDX_Text(pDX, IDC_EDIT_SAMPLEPOINT, m_strSamplePoint);
	DDX_Text(pDX, IDC_EDIT_DIMENSION, m_strDimension);
	DDX_Text(pDX, IDC_EDIT_CODEBOOK, m_strCodeBookFilename);
	DDX_Text(pDX, IDC_EDIT_GAINSAMPLEPOINT, m_strGainSamplePoint);
	DDX_Text(pDX, IDC_EDIT_GAINDIMENSION, m_strGainDimension);
	DDX_Text(pDX, IDC_EDIT_GAINCODEBOOK, m_strGainCodeBookFilename);
	DDX_Text(pDX, IDC_EDIT_Y, m_strY);
	DDX_Control(pDX, IDC_BTN_START, m_BtnStart);
	DDX_Control(pDX, IDC_BTN_END, m_BtnEnd);
	DDX_Text(pDX, IDC_EDIT_STATUS, m_strStatus);
	DDX_Control(pDX, IDC_BTN_RESET, m_BtnReset);
	DDX_Control(pDX, IDC_BTN_CLEAR, m_BtnClear);
	DDX_Control(pDX, IDC_LIST_SHOWDEVICE, m_ListShowDevice);
	DDX_Control(pDX, IDC_BTN_SEARCHDEVICE, m_BtnSearchDevice);
}

BEGIN_MESSAGE_MAP(CECGDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_OpenBTSPP, &CECGDlg::OnBnClickedOpenBTSPP)
	ON_BN_CLICKED(IDC_BTN_CloseBTSPP, &CECGDlg::OnBnClickedCloseBTSPP)
	ON_BN_CLICKED(IDC_BTN_WIFICONNECT, &CECGDlg::OnBnClickedWIFIConnect)
	ON_BN_CLICKED(IDC_BTN_WIFIDISCONNECT, &CECGDlg::OnBnClickedWIFIDisconnect)
	ON_BN_CLICKED(IDC_BTN_PHONECONNECT, &CECGDlg::OnBnClickedPhoneConnect)
	ON_BN_CLICKED(IDC_BTN_PHONEDISCONNECT, &CECGDlg::OnBnClickedPhoneDisconnect)
	ON_BN_CLICKED(IDC_BTN_START, &CECGDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BTN_END, &CECGDlg::OnBnClickedEnd)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CECGDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_BTN_RESET, &CECGDlg::OnBnClickedReset)
	ON_BN_CLICKED(IDC_BTN_SEARCHDEVICE, &CECGDlg::OnBnClickedSearchDevice)
END_MESSAGE_MAP()


// CECGDlg message handlers

BOOL CECGDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_ComboPort.SetCurSel(0);       // The initial serial port is COM2
    m_ComboBaudRate.SetCurSel(0);   // Initial bits per second transmission rate is set at 38400
    m_ComboDataBit.SetCurSel(0);    // Initial data bit is set at 8
    m_ComboParity.SetCurSel(0);     // Initial parity bit is set to none
    m_ComboStopBit.SetCurSel(0);    // Initial stop bit is set to 1
	
	m_BtnCloseBTSPP.EnableWindow(FALSE); // Disable "Close BTHSPP" button
	m_BtnWIFIDisconnect.EnableWindow(FALSE);
	m_BtnPhoneDisconnect.EnableWindow(FALSE);

	m_BtnEnd.EnableWindow(FALSE);

	m_ExitBTHThreadEvent = NULL;
	m_ExitVQThreadEvent = NULL;
	m_ExitWIFIThreadEvent = NULL;
    m_hComm = INVALID_HANDLE_VALUE; // Set BTH serial port device handle as invalid
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CECGDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_ECG_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_ECG_DIALOG));
	}
}
#endif

void CECGDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	if (m_ExitBTHThreadEvent != NULL)
	{
		SetEvent(m_ExitBTHThreadEvent);
		Sleep(1000);
		CloseHandle(m_ExitBTHThreadEvent);
		m_ExitBTHThreadEvent = NULL;
	}
	
	if (m_ExitVQThreadEvent != NULL)
	{
		SetEvent(m_ExitVQThreadEvent);
		Sleep(1000);
		CloseHandle(m_ExitVQThreadEvent);
		m_ExitVQThreadEvent = NULL;
	}

	if (m_ExitWIFIThreadEvent != NULL)
	{
		SetEvent(m_ExitWIFIThreadEvent);
		Sleep(1000);
		CloseHandle(m_ExitWIFIThreadEvent);
		m_ExitWIFIThreadEvent = NULL;
	}

	if (m_ExitPhoneThreadEvent != NULL)
	{
		SetEvent(m_ExitPhoneThreadEvent);
		Sleep(1000);
		CloseHandle(m_ExitPhoneThreadEvent);
		m_ExitPhoneThreadEvent = NULL;
	}

	//delete VQ codeword, don't use vqcb.DeleteCodeWord
	INT i;
	for( i = 0 ; i < vqcb.dimension ; i++)
	{
		if(vqcb.codeWord[i] != NULL)
		{
			delete [] vqcb.codeWord[i];
			vqcb.codeWord[i] = NULL;
		}
	}
	if(vqcb.codeWord != NULL)
	{
		delete [] vqcb.codeWord;
		vqcb.codeWord = NULL;
	}
	
	DeleteCriticalSection(&m_csBTHDataBuf);
	DeleteCriticalSection(&m_csCompressedDataBuf);
	DeleteCriticalSection(&m_csGainBuf);
}

void CECGDlg::OnBnClickedClear()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	//Display status
	UpdateStatus(L"",CLEARSTATUS);
	/*this->m_strStatus = _T("");
	CEdit * pRecvStrEdit = (CEdit *)this->GetDlgItem(IDC_EDIT_STATUS);
	pRecvStrEdit->SetWindowTextW(this->m_strStatus);*/
}

void CECGDlg::UpdateStatus(CString str, BOOL flag)
{
	if(CLEARSTATUS == flag)
		m_strStatus = "";
	this->m_strStatus += str;
	this->m_strStatus += "\r\n";
	CEdit * pRecvStrEdit = (CEdit *)this->GetDlgItem(IDC_EDIT_STATUS);
	pRecvStrEdit->SetWindowTextW(this->m_strStatus);
}

void CECGDlg::OnBnClickedReset()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("Serial port is not open!"));
		return;
	}
	m_BtnReset.EnableWindow(FALSE);
	DWORD dwactlen;
	//Important to delay 100 miniseconds.
	Sleep(100);
	WriteFile(m_hComm, "W", 1, &dwactlen, NULL);
	Sleep(100);
	WriteFile(m_hComm, "W-\r", 3, &dwactlen, NULL);
	Sleep(100);
	WriteFile(m_hComm, "RS\r", 3, &dwactlen, NULL);
	Sleep(100);
	WriteFile(m_hComm, "INIT\r", 5, &dwactlen, NULL);
	Sleep(100);
	m_BtnReset.EnableWindow(TRUE);
}
