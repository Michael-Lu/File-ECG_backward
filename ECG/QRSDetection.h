#ifndef QRSDETECTION_H
#define QRSDETECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>

class QRSDetection
{
private:
	static const int LPBUFSIZE = 12;
	static const int HPBUFSIZE = 32;
	static const int SLOPEBUFSIZE = 5;

	double lp_y0;
	double lp_y1;
	double lp_y2;
	double lpRecord[LPBUFSIZE];
	
	double hp_y0;
	double hp_y1;
	double hpRecord[HPBUFSIZE];

	double slopeBuf[SLOPEBUFSIZE];

	void findR(double *data, DWORD dataLen, unsigned int startIndex, unsigned int endIndex, double &RValue, unsigned int &RIndex);
	double calculate_abs(double v1, double v2);

	double defaultMaxSlope;
public:
	QRSDetection();
	~QRSDetection(){}

	void detectAlg(double *&data, DWORD &beginIndex, DWORD dataLen, int *&RIndex, int &RIndexCnt, int &lastQRSOnsetIndex, int prevQRSOnsetIndex, bool &onSetButNoR);

	double QRSFilter(double data, bool init);
	double lpfilter(double datum, bool init);
	double hpfilter(double datum, bool init);
	double calculateSlope(double datum, bool init);

	void set_defaultMaxSlope(double s){defaultMaxSlope = s;}
	double get_defaultMaxSlope(){return defaultMaxSlope;}
};

/*void QRSDetection(double *&data, DWORD &beginIndex, DWORD dataLen, int *&RIndex, int &RIndexCnt, int &lastQRSOnsetIndex, int prevQRSOnsetIndex, bool &onSetButNoR);

double QRSFilter(double, bool);
double lpfilter(double, bool);
double hpfilter(double, bool);
double calculateSlope(double, bool);
void findR(double *, DWORD , unsigned int startIndex, unsigned int endIndex, double &RValue, unsigned int &RIndex);
double calculate_abs(double v1, double v2);*/

#endif
