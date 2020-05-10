#include <time.h>

typedef struct timespec timer;
static timer timer_now()
{
	timer t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t;
}
static float timer_time(const timer *start)
{
	timer end = timer_now();
	int s = (int)end.tv_sec - (int)start->tv_sec;
	int f = (int)end.tv_nsec - (int)start->tv_nsec;
	if(f < 0)
	{		
	   --s;
	   f += 1000000000;
	}
	return (float)((double)s + (double)(f / 1000000000.0));
}




