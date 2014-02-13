#include <cmath>
#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include "QRSDetection.h"

//const double thresh_param = 8; //threshold for 100.mat
//const double thresh_param = 2.0;// threshold for sensor simulation signals
const double thresh_param = 2.0;// threshold for WY Tsai's signals
const int filter_param = 6;
//const double init_maxi = 2.795; //for 100.mat
//const double init_maxi = 75723; //for simualtion signal(from BTECG)
//const double init_maxi = 58893; //for WY Tsai's signal(from BTECG)
//const double init_maxi = 59355; //for simualtion signal(from BTECG)
const double QRSOnsetLen = 2;
const DWORD delay = 23;

using namespace std;

void QRSDetection::detectAlg(double *&data, DWORD &beginIndex, DWORD dataLen, int *&RIndexArray, int &RIndexCnt, int &lastQRSOnsetIndex, int prevQRSOnsetIndex, bool &onSetButNoR)
{
	//memset(RIndexArray, 0, sizeof(int) * cPeriodNum);
	//RIndexCnt = 0;

	prevQRSOnsetIndex = -1;
	lastQRSOnsetIndex = -1;

	DWORD i;
	double datum;
	double slope;
	double slope_thresh;

	//static double maxi = init_maxi;
	static double maxi = defaultMaxSlope;
	static double firstMax = 0.0;
	static int findOnSet_cnt = 0;
	static bool QRSOnset = false;
	static bool QRSBufInit = true;

	double RValue;
	unsigned int RIndex = 0;

	if( onSetButNoR == true )
	{
		prevQRSOnsetIndex = beginIndex;
		lastQRSOnsetIndex = beginIndex;
		onSetButNoR = false;
		findR(data, dataLen, beginIndex, beginIndex + 100, RValue, RIndex);
		firstMax = calculate_abs(RValue, data[prevQRSOnsetIndex]);
		RIndexArray[RIndexCnt++] = RIndex;
	}

	slope_thresh = (thresh_param/16) * maxi;

	if( QRSBufInit == true )
	{
		lpfilter(0, true);
		hpfilter(0, true);
		calculateSlope(0, true);
		QRSBufInit = false;
	}

	static bool first = true;
	//static unsigned int prevQRSOnsetIndex = 0;
	for( i = beginIndex ; i < dataLen ; i++ )
	{
		datum = QRSFilter(data[i], false);
		slope = calculateSlope(datum, false);
		slope_thresh = (thresh_param/16) * maxi;
		if( slope > slope_thresh )
		{
			if( findOnSet_cnt <= QRSOnsetLen )
				findOnSet_cnt++;
			if( findOnSet_cnt > QRSOnsetLen )
			{
				if( (i-delay) <= (prevQRSOnsetIndex+100))
				{
					if( first == false )
						continue;
				}
				prevQRSOnsetIndex = i-delay;
				if(prevQRSOnsetIndex < 0)
					continue;
				findOnSet_cnt = 0;
				first = false;
				lastQRSOnsetIndex = i-delay;
				if( (prevQRSOnsetIndex + 100) < dataLen)
				{
					findR(data, dataLen, prevQRSOnsetIndex, prevQRSOnsetIndex + 100, RValue, RIndex);
					firstMax = calculate_abs(RValue, data[prevQRSOnsetIndex]);
					RIndexArray[RIndexCnt++] = RIndex;
				}
				else
				{
					onSetButNoR = true;
					beginIndex = i - delay + 1 - RIndexArray[RIndexCnt - 1];
					break;
				}
			}
		}
		else
		{
			findOnSet_cnt = 0;
			//QRSOnset = false;
		}
	}
}

QRSDetection::QRSDetection()
{
	//set low-pass filter
	lp_y0 = 0;
	lp_y1 = 0;
	lp_y2 = 0;
	lpRecord[LPBUFSIZE];
	memset(lpRecord, 0, LPBUFSIZE * sizeof(double));

	//set high-pass filter
	hp_y0 = 0;
	hp_y1 = 0;
	memset(hpRecord, 0, HPBUFSIZE * sizeof(double));

	//set slope buffer
	memset(slopeBuf, 0, SLOPEBUFSIZE * sizeof(double));

	defaultMaxSlope = 0.0;
}

double QRSDetection::QRSFilter(double data, bool init)
{
	double datum;

	datum = lpfilter(data, init);
	datum = hpfilter(datum, init);

	return datum;
}

double QRSDetection::lpfilter(double datum, bool init)
{
	if(init == true)
	{
		lp_y0 = 0;
		lp_y1 = 0;
		lp_y2 = 0;
		memset(lpRecord, 0, LPBUFSIZE * sizeof(double));
		return -1;
	}

	lp_y0 = (lp_y1 * 2) - lp_y2 + datum - ((lpRecord[5]) * 2) + lpRecord[11];
	lp_y2 = lp_y1;
	lp_y1 = lp_y0;
	memmove(lpRecord + 1, lpRecord, (LPBUFSIZE-1) * sizeof(double));
	lpRecord[0] = datum;

	lp_y0 /= 32;

	return lp_y0;
}

double QRSDetection::hpfilter(double datum, bool init)
{
	if(init == true)
	{
		hp_y0 = 0;
		hp_y1 = 0;
		memset(hpRecord, 0, HPBUFSIZE * sizeof(double));
		return -1;
	}

	hp_y0 = (hpRecord[15] * 32) - ((hp_y1 + datum - hpRecord[31]) / 32);
	hp_y1 = hp_y0;
	memmove(hpRecord + 1, hpRecord, (HPBUFSIZE-1) * sizeof(double));
	hpRecord[0] = datum;

	return hp_y0;
}

double QRSDetection::calculateSlope(double datum, bool init)
{
	double slope = 0.0;

	if(init == true)
	{
		memset(slopeBuf, 0, SLOPEBUFSIZE * sizeof(double));
		return -1;
	}

	memmove(slopeBuf + 1, slopeBuf, (SLOPEBUFSIZE-1) * sizeof(double));
	slopeBuf[0] = datum;
	slope = (-2) * slopeBuf[4] - slopeBuf[3] + slopeBuf[1] + 2 * slopeBuf[0];
	slope /= 2;

	return slope;
}

void QRSDetection::findR(double *data, DWORD dataLen, unsigned int startIndex, unsigned int endIndex, double &RValue, unsigned int &RIndex)
{
	unsigned int i;
	RValue = data[startIndex];
	RIndex = startIndex;

	if(endIndex >= dataLen)
		endIndex = dataLen - 1;
	
	for( i = startIndex + 1 ; i <= endIndex ; i++ )
	{
		if( RValue < data[i] )
		{
			RValue = data[i];
			RIndex = i;
		}
	}
}

double QRSDetection::calculate_abs(double v1, double v2)
{
	if( v1 > v2 )
		return v1 - v2;
	else
		return v2 - v1;
}