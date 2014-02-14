#include <cstdio>
#include <windows.h>
/*
This function is intended to be called from GetECGRawData(). Its purpose is to simulate the *.txt testing file as the bluetooth ECG sensor. When using it, the codes for parsing the received data from bluetooth
ECG sensor should be disabled. The rest part of whole project still call GetECGRawData() as an interface as if nothing changed. 
Note that data in the txt file named by (char* srcfile) should be separated by whitespace. 
*/

int ReadFromFile(char *srcfile, double *&data, DWORD &dataLen){

	
	static bool init = false;
	static FILE *fp = NULL;
	
	
	int n;
	int err = 0;
	
	if(dataLen > 512*4-1){ //data pointed to a memory block with size of 512*4 bytes
		return -1;
	}

	if(init == false ){
		fp = fopen(srcfile,"r");
		init = true;
	}

	
	if(fp == NULL){
		err = -1;
		init = false;
		return err;
	}

//if(init == false){

	for(int count = 0; count<32; count++){
		
		n = fscanf(fp,"%lf ", &data[dataLen++]);
		

		if( feof(fp) ){
			err = 0;
			goto CleanUp;
			break;
		}

		if(n!=1){
			err = -1;
			goto CleanUp;
			break;
		}

	}
//}else{
//	char word;
//	n = fread(&word,sizeof(char),1,fp);
//}



	//Sleep(100);




	return err;
CleanUp:
	init = false;
	fclose(fp);
	fp = NULL;

	return err;
}
