#include <cmath>
#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include "dsc.h"

const int SIZEOFCHAR = 8;

void dsc::encoder(char *inArray, int inLen, char *outputArray, int &outputLen)
{
	int i;
	int byteIndex = SIZEOFCHAR - 1;
	unsigned int byte = 0;
	outputLen = 0;
	
	if( YisSent == false )
	{
		inArray += vectorPerCycle;
		inLen -= vectorPerCycle;
		YisSent = true;
	}
	for( i = 0 ; i < inLen ; i++ )
	{
		if( i % vectorPerCycle == 0 )
		{
			regD1 = 0;
			regD2 = 0;
		}
		in1 = int(inArray[i]) & IN1MASK;
		in2 = int(inArray[i]) & IN2MASK;
		in3 = int(inArray[i]) & IN3MASK;
		in4 = int(inArray[i]) & IN4MASK;
		in5 = int(inArray[i]) & IN5MASK;
		in6 = int(inArray[i]) & IN6MASK;

		in1 = in1 >> 5;
		in2 = in2 >> 4;
		in3 = in3 >> 3;
		in4 = in4 >> 2;
		in5 = in5 >> 1;

		wire1 = in1 ^ regD1;
		//outputArray[outputLen++] = char(wire1 ^ in2 ^ in5);
		byte += unsigned int(wire1 ^ in2 ^ in5) * unsigned int(pow(2, byteIndex));
		byteIndex--;
		regD1 = wire1 ^ in3 ^ in4;
		wire2 = regD2 ^ in1 ^ in2 ^ in3;
		//outputArray[outputLen++] = char(wire2 ^ in4 ^ in6);
		byte += unsigned int(wire2 ^ in4 ^ in6) * unsigned int(pow(2, byteIndex));
		byteIndex--;
		regD2 = wire2;

		if(byteIndex == -1)
		{
			byteIndex = SIZEOFCHAR - 1;
			outputArray[outputLen++] = char(byte);
			byte = 0;
		}
	}


}