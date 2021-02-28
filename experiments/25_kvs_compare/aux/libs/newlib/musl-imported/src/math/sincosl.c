#define _GNU_SOURCE

#include <math.h>

void sincosl(long double x, long double *sin, long double *cos)
{
	double sind, cosd;

	sincos(x, &sind, &cosd);
	*sin = sind;
	*cos = cosd;
}
