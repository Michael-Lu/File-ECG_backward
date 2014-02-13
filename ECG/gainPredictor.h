#ifndef GAINPREDICTOR_H
#define GAINPREDICTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include "constants.h"

class gainPredictor
{
private:
	double c1;
	double c2;
	double gain;
	double *x_recon;
	double computeVectorNorm(double *d, int len);
	const double default_sim_gain;
public:
	double firstGain;
	gainPredictor(double alpha = 0.45):default_sim_gain(6100)
	{
		c1 = alpha;
		c2 = 1 - alpha;
		x_recon = new double [vqcb.samplePoint];
	}
	~gainPredictor() {delete [] x_recon;}
	double computeGain(bool &firstGainCalculated, double *x, double *x_reconstructive, int len);
	double computeNormGain(double *x, int len);
};

#endif