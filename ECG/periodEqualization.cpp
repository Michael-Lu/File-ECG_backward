#include <cmath>
#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include "periodEqualization.h"

void periodEqualization::periodNormalize(double *pData, double *pNormarlizedData, int *originalDataLen,int originalNumofCycles, int normalizedLen, int &normalizedDataTotalLen)
{
	//linear equation: ax+by=0
	data = pData;
	normarlizedData = pNormarlizedData;
	
	int xi_nominator = normalizedLen - 1;
	int xi_denominator = normalizedLen - 1;
	int interval;

	int i, j;
	double a, b;
	int offset = 0;

	int beginIndex = 0;
	int checkEndPoint = normalizedLen - 1;
	//FILE *ofp = fopen("nor.txt", "a");

	for( i = 0 ; i < originalNumofCycles ; data += originalDataLen[i], i++)
	{
		interval = 0;
		xi_nominator = normalizedLen - 1;
		xi_denominator = normalizedLen - 1;
		for( j = 0 ; j < normalizedLen ; j++, xi_nominator += (originalDataLen[i]-1))
		{
			findInterval(xi_nominator, xi_denominator, interval, originalDataLen[i], normalizedLen);
			if( j == checkEndPoint)
				processingData[j] = data[originalDataLen[i]-1];
			else
			{
				a = (data[interval+1]-data[interval]);
				b = (data[interval]*(double(interval+2))-data[interval+1]*(double(interval+1)));
				processingData[j] = ((double)xi_nominator/xi_denominator) * a + b;
			}
			//fprintf(ofp, "%d, %04lf\n", interval, processingData[j]);
		}
		memcpy(pNormarlizedData+(i*normalizedLen), processingData, sizeof(double)*normalizedLen);
	}
	normalizedDataTotalLen = originalNumofCycles * normalizedLen;
	//fclose(ofp);
}
void periodEqualization::findInterval(int xi_nominator, int &xi_denominator, int &interval, int dataLen, int normalizedLen)
{
	//int interval is zero-based.
	int xi = xi_nominator/xi_denominator;
	if( xi == dataLen )
		interval = (dataLen - 2);
	else
		interval = (xi-1);
}