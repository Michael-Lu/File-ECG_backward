#include "stdafx.h"
#include "tmpnam.h"
#include <stdio.h>
#include <string.h>

char* tmpnam(char* str){
	
	static char _str[L_tmpnam] = {'A','A','@','\0'};
	short i;
	
	_str[L_tmpnam-2] += 1;
	
	

	for(i = L_tmpnam-2; i>0 ; i--){

		if( _str[i] == ('Z'+1) ){
			
			_str[i] = 'A';
			_str[i-1] += 1;
		
		}else{

			continue;
		}
	}

	

	if( _str[0] == 'Z'+1 ){ /* reach TMP_MAX */
			_str[0] ='A';

	}
	
	if( str == NULL ){
		return _str;

	}else{
		strcpy(str,_str);		
		return 0;

	}

}