#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include "VQClass.h"

class quicksortClass
{
private:
	struct st_distance_index *m_data;
	void myquicksort(struct st_distance_index *&input, int p, int r);
	int randomized_partition(struct st_distance_index *&a, int p, int r);
	void pick_pivot(struct st_distance_index *&a, int p, int r);
public:
	int randomSelect(struct st_distance_index *&input, int p, int r, int ith);

friend class VQClass;
};


#endif