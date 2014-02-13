#ifndef CONSTANTS_H
#define CONSTANTS_H

/*
#define Debug_PrintShapeCodebook
#define Debug_PrintGainCodebook
#define Debug_PrintVQIndex
#define Debug_PrintGainValue
*/

#define Debug
#ifdef Debug

//#define Debug_PrintPeriodNormalizedData

//#define Debug_PrintPeriodLength
//#define Debug_DumpScaledECG
//#define Debug_DumpEncodedECG /*dump encoded ECG JP2 Image*/

//#define Debug_PrintRIndex  //Useless now, need to be recovered. The actual retrieving R index codes disappeared!
//#define Debug_PrintECGRawData

#endif

const CString PortTbl[6] = {_T("COM4:"), _T("COM5:"), _T("COM6:"), _T("COM7:"), _T("COM8:"), _T("COM9:")};
const DWORD BaudTbl[1] = {CBR_19200};
const DWORD DataBitTbl[1] = {8};
const BYTE ParityTbl[1] = {NOPARITY};
const BYTE StopBitTbl[1] = {ONESTOPBIT};

const int cPeriodNum = 5; /*data(from ECG sensor) should contains 5 (or less) cycles.*/
const int cNormalizedLen = 200; /*the point number of period normalized data*/
const int vectorPerCycle = cNormalizedLen / 4;
const int cNormalizedCnt = cPeriodNum-1;   /*only four group of normalized data. 4 is come from 5(cPeriodNum) minus 1.*/

const int cBTH_Shared_Mem_Size_PeriodCnt = 15;
const int cBTHSharedMem_Read_LeastPeriodCnt = 10; //retrieve 10 Periods data from SharedMem_BTHDataBuf per reading action
const int cnumofSelectedIndex = 5; //for multi-choice
const double default_alpha = 0.45;
    

#endif
