#ifndef PERIODEQUALIZATION_H
#define PERIODEQUALIZATION_H

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include "constants.h"

class periodEqualization
{
private:
	double *data;
	double *normarlizedData;
	double *processingData;
public:
	periodEqualization()
	{
		processingData = new double [cNormalizedLen];
		if( processingData == NULL )
		{
			FILE *fp = fopen("err.txt", "a");
			fprintf(fp, "Error in class periodEqualization: create processingData fault\n");
			fclose(fp);
			return;
		}
	}
	~periodEqualization()
	{
		if( processingData != NULL )
		{
			delete [] processingData;
			processingData = NULL;
		}
	}
	void periodNormalize(double *pData, double *pNormarlizedData, int *originalDataLen,int originalNumofCycles, int normalizedLen, int &normalizedDataTotalLen);
	void findInterval(int xi_nominator, int &xi_denominator, int &interval, int dataLen, int normalizedLen);
};

#endif