#ifndef DSC_H
#define DSC_H

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include "constants.h"

class dsc
{
private:
	const int IN1MASK;
	const int IN2MASK;
	const int IN3MASK;
	const int IN4MASK;
	const int IN5MASK;
	const int IN6MASK;
	const int INPUTBITSIZE;
	const int OUTPUTBITSIZE;

	int wire1;
	int wire2;
	int regD1;
	int regD2;

	int in1;
	int in2;
	int in3;
	int in4;
	int in5;
	int in6;

	int output1;
	int output2;

	bool YisSent;
public:
	dsc(int samplePoint, int outputBitNum):
		IN1MASK(32),
		IN2MASK(16),
		IN3MASK(8),
		IN4MASK(4),
		IN5MASK(2),
		IN6MASK(1),
		INPUTBITSIZE(samplePoint),
		OUTPUTBITSIZE(outputBitNum),
		wire1(0),
		wire2(0),
		regD1(0),
		regD2(0),
		YisSent(false)
		{}
	void encoder(char *inArray, int inLen, char *outputArray, int &outputLen);
	
};

#endif