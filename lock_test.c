#include "atomic.h"
#include "timer.h"
#include <threads.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

static atomic(int) wait;
static thrd_t threads[8192];
static spinlock lock1;
static uint64_t count;
static int cpus;
static int run(void *ctx)
{
	while(!wait) ;	

	while(wait)
	{
		lock(&lock1);
		++count;
		unlock(&lock1);
		//if(cpus >= 2048) printf("count=%lu\n", count);
	}
	return 0;
}


#include <unistd.h>

int main(int argc, char** argv)
{
	for(int i=1;i<=8192;i*=2)
	{
		cpus = i;
		wait = 0;
		count = 0;
		lock_init(&lock1);
//		mtx_init(&lock1, mtx_plain);
//		memset(&lock1, 0, sizeof(lock1));
		for(int j=0;j<i;j++)		
			thrd_create(&threads[j], run, 0);	
//		sleep(1);	
		timer t = timer_now();
		wait = 1;
		int rc;
		while(timer_time(&t) < 1){
			struct timespec s = {0, 1};
		       	nanosleep(&s, 0);		
		}
		wait = 0;
		//printf("joining...\n");
		for(int j=0;j<i;j++)
		{
		//      printf("joined %d threads\n", j);
			thrd_join(threads[j], &rc);			
		}
		fprintf(stderr, "%d %f %lu\n", i, timer_time(&t), count);
	}
}

