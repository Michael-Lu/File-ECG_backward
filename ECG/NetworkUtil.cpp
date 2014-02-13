#include "stdafx.h"
#include "ECG.h"
#include "NetworkUtil.h"


INT GetBA(WCHAR **pp, BT_ADDR *pba)
{
	while(**pp == ' ')
		++*pp;

	int i;
	for( i = 0 ; i < 4 ; ++i, ++*pp)
	{
		if(! iswxdigit (**pp))
			return FALSE;

		int c = **pp;
		if (c >= 'a')
			c = c - 'a' + 0xa;
		else if(c >= 'A')
			c = c - 'A' + 0xa;
		else c = c - '0';
		if((c < 0) || (c > 16))
			return FALSE;
		*pba = *pba * 16 + c;
	}

	for (i = 0 ; i < 8 ; ++i, ++*pp)
	{
		if(! iswxdigit (**pp))
			return FALSE;
		int c = **pp;
		if(c >= 'a')
			c = c - 'a' + 0xa;
		else if(c >= 'A')
			c = c - 'A' + 0xa;
		else c = c - '0';
		if((c < 0) || (c > 16))
			return FALSE;
		*pba = *pba * 16 + c;
	}

	if((**pp != ' ') && (**pp != '\0'))
		return FALSE;

	return TRUE;
}

INT GetDI(WCHAR **pp, unsigned int *pi)
{
	while(**pp == ' ')
		++*pp;

	int iDig = 0;
	*pi = 0;
	while(iswdigit (**pp))
	{
		int c = **pp;
		c = c - '0';
		if((c < 0) || (c > 9))
			return FALSE;
		*pi = *pi * 10 + c;
		++*pp;
		++iDig;
	}

	if((iDig <= 0) || (iDig > 10))
		return FALSE;

	return TRUE;
}
