#include <cmath>
#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include "gainPredictor.h"

double gainPredictor::computeGain(bool &firstGainCalculated, double *x, double *codevector, int len)
{
	//這裡的codevector是查表後得到的向量，要先乘回去才能predict gain
	int i;

	if(firstGainCalculated == true)
	{
		//gain = computeVectorNorm(x, len);
		gain = default_sim_gain;
		firstGain = gain;
		firstGainCalculated = false;
	}
	else
	{
		for( i = 0 ; i < len ;  i++ )
			x_recon[i] = codevector[i] * gain;
		gain = c1 * gain + c2 * computeVectorNorm(x_recon, len);
	}
	return gain;
}

double gainPredictor::computeNormGain(double *x, int len)
{
	gain = computeVectorNorm(x, len);
	return gain;
}

double gainPredictor::computeVectorNorm(double *d, int len)
{
	int i;
	double norm;
	double sum = 0;
	for( i = 0 ; i < len ; i++)
		sum += pow(d[i], 2);
	norm = sqrt(sum);
	return norm;
}