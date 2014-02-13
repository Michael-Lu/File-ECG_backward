#ifndef CODEBOOK_H
#define CODEBOOK_H

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include "ECGDlg.h"

class VQCodeBook
{
public:
	INT M;
    INT dimension;
    INT samplePoint;
    double **codeWord;
};

extern VQCodeBook vqcb;

#endif
