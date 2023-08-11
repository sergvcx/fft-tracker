#include "nmtype.h"

int comp_imax(int N, nm32fcr *X, int INCX)
{
	int indx, i;
	float max = 0;
	indx = 0;
	for(i = 0; i < N; i++) {
		if(max < X[i].re) {
			max = X[i].re;
			indx = i;
		}
	}
	return indx;
}