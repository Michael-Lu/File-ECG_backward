#ifndef VQCLASS_H
#define VQCLASS_H

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include "ECGDlg.h"
#include "CodeBook.h"
#include "GainCodebook.h"
#include "quicksort.h"
#include "constants.h"

struct st_distance_index
{
	double data;
	int index;
};

class VQClass
{
private:
	quicksortClass qsort;

	const int tableLen;
	int YIndex;
	int bitNum;
	int vectorPerCycle;
	struct st_distance_index *index_array;
	struct st_distance_index *index_array_back;
	int *min_dist_idx5;
	int *VQ_without_communication_index_Y;

	bool init_table(int tableLen, int bitNum);
	bool init_Y(CString filename, int len);
	void dec2bin(int decimal, int *binary, int len);
	int summate_1D_Array_Element(int *arr, int len);
	inline double computeDiffofSquare(double &v, double &v2)
	{
		double v1 = v;
		return (v2-v1)*(v2-v1);
	}
	bool search(int key, int *data, int data_len);
public:
	int *table;

	void LookUpCodeBook(double *, INT &);
	void LookUpGainCodeBook(double *, INT &);
	void showBitTable(int len);
	void showY();
	double *getACodeBookEntry(int index);
	VQClass(int len, int n, int s, CString YFilename);
	~VQClass();
	int getVectorPerCycle(){return vectorPerCycle;}
};


#endif
