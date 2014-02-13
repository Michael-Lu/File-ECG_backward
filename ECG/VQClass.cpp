#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include "VQClass.h"
#include "constants.h"

VQClass::VQClass(int len, int n, int s, CString YFilename)
		:tableLen(len),
		vectorPerCycle(cNormalizedLen / s)
{
	bool ret;

	bitNum = n;
	YIndex = 0;

	table = new int [len];
	if( table == NULL )
	{
		FILE *fp = fopen("err.txt", "a");
		fprintf(fp, "Error in class VQClass: create table fault\n");
		fclose(fp);
		return;
	}
	index_array = new struct st_distance_index [vqcb.dimension];
	if( index_array == NULL )
	{
		FILE *fp = fopen("err.txt", "a");
		fprintf(fp, "Error in class VQClass: create index_array fault\n");
		fclose(fp);
		return;
	}
	index_array_back = new struct st_distance_index [vqcb.dimension];
	if( index_array_back == NULL )
	{
		FILE *fp = fopen("err.txt", "a");
		fprintf(fp, "Error in class VQClass: create index_array_back fault\n");
		fclose(fp);
		return;
	}
	min_dist_idx5 = new int [cnumofSelectedIndex];
	if( min_dist_idx5 == NULL )
	{
		FILE *fp = fopen("err.txt", "a");
		fprintf(fp, "Error in class VQClass: create min_dist_idx5 fault\n");
		fclose(fp);
		return;
	}

	VQ_without_communication_index_Y = new int [vectorPerCycle];
	if( VQ_without_communication_index_Y == NULL )
	{
		FILE *fp = fopen("err.txt", "a");
		fprintf(fp, "Error in class VQClass: create VQ_without_communication_index_Y fault\n");
		fclose(fp);
		return;
	}

	ret = init_table(tableLen, bitNum);
	if(ret == false)
	{
		FILE *fp = fopen("err.txt", "a");
		fprintf(fp, "Error in class VQClass: initial table fault\n");
		fclose(fp);
		return;
	}

	ret = init_Y(YFilename, vectorPerCycle);
	if(ret == false)
	{
		FILE *fp = fopen("err.txt", "a");
		fprintf(fp, "Error in class VQClass: initial Y fault\n");
		fclose(fp);
		return;
	}
}
VQClass::~VQClass()
{
	if(table != NULL)
	{
		delete [] table;
		table = NULL;
	}

	if(index_array != NULL)
	{
		delete [] index_array;
		index_array = NULL;
	}

	if(index_array_back != NULL)
	{
		delete [] index_array_back;
		index_array_back = NULL;
	}

	if(min_dist_idx5 != NULL)
	{
		delete [] min_dist_idx5;
		min_dist_idx5 = NULL;
	}

	if(VQ_without_communication_index_Y != NULL)
	{
		delete [] VQ_without_communication_index_Y;
		VQ_without_communication_index_Y = NULL;
	}
}

void VQClass::LookUpCodeBook(double *data, INT &index)
{
	double min_d = 0;
    double cur_d = 0;
    int i, j;

	int returnVal = 0;

	for( j = 0 ; j < vqcb.samplePoint ; j++ )
    {
        min_d += computeDiffofSquare(data[j], vqcb.codeWord[0][j]);
    }
	for( i = 1 ; i < vqcb.dimension ; i++ )
    {
        cur_d = 0;
		for( j = 0 ; j < vqcb.samplePoint ; j++ )
        {
            cur_d += computeDiffofSquare(data[j], vqcb.codeWord[i][j]);
        }
		if( cur_d < min_d )
		{
			min_d = cur_d;
			returnVal = i;
		}
	}
	index = returnVal;
}

void VQClass::LookUpGainCodeBook(double *data, INT &index)
{
    int min_idx = 0;
    double min_d = 0;
    double cur_d = 0;
    int i, j;

    //compute the first difference of square
    for( i = 0 ; i < gcb.samplePoint ; i++ )
        min_d += computeDiffofSquare(data[i], gcb.codeWord[0][i]);
    ////////////////////////////////////////

    for( i = 1; i < gcb.dimension ; i++ )
    {
        cur_d = 0;
        for( j = 0 ; j < gcb.samplePoint ; j++ )
        {
            cur_d += computeDiffofSquare(data[j], gcb.codeWord[i][j]);
        }
        if(cur_d < min_d)
        {
            min_d = cur_d;
            min_idx = i;
        }
    }
    index = min_idx;
}

bool VQClass::init_table(int tableLen, int bitNum)
{
	int i;

	int *binary = new int [bitNum];
	if( binary == NULL)
	{
		return false;
	}
	for( i = 0 ; i < tableLen ; i++ )
	{
		dec2bin(i, binary, bitNum);
		table[i] = summate_1D_Array_Element(binary, bitNum);
	}

	delete [] binary;
	return true;
}

bool VQClass::init_Y(CString filename, int len)
{
	int filenameLen = filename.GetLength();
	char *pfilename = new char [filenameLen];
	if(pfilename == NULL)
	{
        return false;
	}
	for(int i = 0 ; i < filenameLen ; i++)
		pfilename[i] = (char)filename.GetAt(i);
	pfilename[filenameLen] = '\0';

	FILE *Yfp = fopen(pfilename, "r");

	if( Yfp == NULL ) 
        return false;
	
	delete [] pfilename;

	for( int i = 0 ; i < vectorPerCycle ; i++)
	{
		fscanf(Yfp, "%d", &VQ_without_communication_index_Y[i]);
	}
	fclose(Yfp);

	return true;
}

void VQClass::dec2bin(int decimal, int *binary, int len)
{
	int i = len -1;
	while(i+1)
	{
		binary[i--] = (1 & decimal);
		decimal >>= 1;
	}
}

int VQClass::summate_1D_Array_Element(int *arr, int len)
{
	int sum = 0;
	int i;
	for( i = 0 ; i < len ; i++ )
		sum += arr[i];
	return sum;
}

void VQClass::showBitTable(int len)
{
	FILE *fp = fopen("bitTable.txt", "w");
	int i;

	for( i = 0 ; i < len ; i++ )
		fprintf(fp, "%d\n", table[i]);

	fclose(fp);
}

void VQClass::showY()
{
	FILE *fp = fopen("readY.txt", "w");
	int i;

	for( i = 0 ; i < vectorPerCycle ; i++ )
		fprintf(fp, "%d\n", VQ_without_communication_index_Y[i]);

	fclose(fp);
}

double *VQClass::getACodeBookEntry(int index)
{
	return vqcb.codeWord[index];
}

bool VQClass::search(int key, int *data, int data_len)
{
	int i;
	for( i = 0 ; i < data_len ; i++ )
	{
		if( key == data[i] )
			return true;
	}
	return false;
}
