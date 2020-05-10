#include "atomic.h"
#include "timer.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

static atomic(int) wait;
static thrd_t threads[8192];
static atomic(uint64_t) count;
static int cpus;
static int run(void *ctx)
{
	while(!wait) ;	

	int c = 0;
	PoolElement *array[65536];
	int a = 0;	
	while(wait)
	{
//		PoolElement *e = pool_alloc();
		PoolElement *e = malloc(sizeof(PoolElement));
		array[a++] = e;
		++count;
//		atomic_fetch_add(count, 1);	
		e->size = 10;
		c += e->size;
		if(a == 65536)
		{
			for(int i=0;i<65536;i++) 
				free(array[i]);
//				pool_free(array[i]);
			a = 0;
		}
//		pool_free(e);		
//		free(e);
		//if(cpus >= 2048) printf("count=%lu\n", count);
	}
	return (int)c;
}


#include <unistd.h>

int main(int argc, char** argv)
{
	for(int i=1;i<=8192;i*=2)
	{
		cpus = i;
		wait = 0;
		count = 0;
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

