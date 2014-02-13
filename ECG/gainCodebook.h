#ifndef GAINCODEBOOK_H
#define GAINCODEBOOK_H

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include "ECGDlg.h"

class gainCodebook
{
public:
	INT M;
    INT dimension;
    INT samplePoint;
    double **codeWord;
};

extern gainCodebook gcb;

#endif
