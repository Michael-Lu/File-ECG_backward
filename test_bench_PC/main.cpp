#include <cstdio>
#include <windows.h>
#include "ReadFromFile.h"

int main(){
	DWORD dataLen=0;
	double *data = new double[512*4];
	int err;

	FILE* fp = fopen("read_from.txt","w+");

	while(1){
		int orig_size = dataLen;
		err = ReadFromFile("mit_test_tru.txt", data, dataLen);
		
		for(short i = orig_size ; i<dataLen ;i++)
			fprintf(fp,"%lf ", data[i]);
		
		if(err == -1)
			break;
	}

	fclose(fp);
	
	delete [] data;
	return 0;
}