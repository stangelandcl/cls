#include "atomic.h"
#include <threads.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

static atomic(int) wait;
static thrd_t threads[4096];
static spinlock lock;
static mtx_t lock2;
static ticketlock lock3;
static uint64_t count;
static int run(void *ctx)
{
	while(!wait) ;	

	while(wait)
	{
//		ticketlock_acquire(&lock3);
		spinlock_acquire(&lock);
//		mtx_lock(&lock2);
		++count;
//		mtx_unlock(&lock2);
		spinlock_release(&lock);
//		ticketlock_release(&lock3);
	}
}


static float calc_seconds(struct timespec *start, struct timespec *end)
{
	    int s = (int)end->tv_sec - (int)start->tv_sec;
	        int f = (int)end->tv_nsec - (int)start->tv_nsec;
		    if(f < 0)
			        {
					            --s;
						                f += 1000000000;
								    }
		        return (float)((double)s + (double)(f / 1000000000.0));
}

#include <unistd.h>

int main(int argc, char** argv)
{
	struct timespec start, end;
	for(int i=1;i<=4096;i*=2)
	{
		wait = 0;
		count = 0;
//		mtx_init(&lock2, mtx_plain);
		memset(&lock, 0, sizeof(lock));
		memset(&lock3, 0, sizeof(lock3));
		for(int j=0;j<i;j++)		
			thrd_create(&threads[j], run, 0);	
		sleep(1);	
		clock_gettime(CLOCK_MONOTONIC, &start);
		wait = 1;
		int rc;
		for(;;)
		{
			clock_gettime(CLOCK_MONOTONIC, &end);
			if(calc_seconds(&start, &end) >= 1) break;
		}
		wait = 0;
		for(int j=0;j<i;j++)
			thrd_join(threads[j], &rc);
		clock_gettime(CLOCK_MONOTONIC, &end);

		fprintf(stderr, "%d %f %lu\n", i, calc_seconds(&start, &end), count);
	}
}

