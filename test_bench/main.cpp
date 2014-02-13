#include <cstdio>
#include "ReadFromFile.h"
#include <windows.h>

int main(){
	DWORD dataLen=0;
	double *data = new double[512*4];
	int err;

	while(1){
		err = ReadFromFile("mit_test.txt", data, dataLen);
		printf("%lu %d\n",dataLen,err);
		
		if(err == -1)
			break;
	}
	
	delete [] data;
	return 0;
}