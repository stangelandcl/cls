#include <stdatomic.h>
#include <threads.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//#include <immintrin.h>

#define atomic(x) _Atomic x

/* Faster than ticket lock. For when it is okay for the same thread to keep acquiring the lock 
 * repeatedly before giving other threads a chance */
typedef struct spinlock
{
//	atomic_flag flag;
	atomic(char) flag;
} spinlock;
static int spinlock_trylock(spinlock *s)
{
	if(atomic_fetch_add(&s->flag, 1) > 1)
	{
		atomic_fetch_add(&s->flag, -1);
		return 0;
	}
	return 1;
}
static void spinlock_lock(spinlock* s)
{	
	char actual;
//	int count = 10;
//	do {
//		if(mtx_trylock(&s->mutex)) return;
//		if(atomic_flag_test_and_set_explicit(&s->flag, memory_order_acquire)) return;//		
//		if(atomic_compare_exchange_strong(&s->flag, &actual, 1)) return;
//		if(spinlock_trylock(s)) return;
//		actual = 0;
//		if(atomic_compare_exchange_strong_explicit(&s->flag, 
//		    &actual, 1, memory_order_acquire, memory_order_acquire)) return;
	//  	else _mm_pause();
//	}while (--count);
//	while(!atomic_flag_test_and_set_explicit(&s->flag, memory_order_acquire))
	for(;;)
	{
		actual = 0;
		if(atomic_compare_exchange_weak_explicit(&s->flag, 
	    &actual, (char)1, memory_order_acquire, memory_order_relaxed)) return;
//	    if(spinlock_trylock(s)) return;
//	while(!atomic_compare_exchange_strong(&s->flag, &actual, 1))	   
		thrd_yield();
	}
}
static void spinlock_unlock(spinlock* s) 
{ 
//	atomic_fetch_add(&s->flag, -1);
//	s->flag = 0;
	atomic_store_explicit(&s->flag, (char)0, memory_order_release);
//	atomic_flag_clear_explicit(&s->flag, memory_order_release);	
}


/* Slower than spin lock. For when threads should be serviced in FIFO order */
typedef struct ticketlock
{
	atomic(int) current;
	atomic(int) next;
} ticketlock;
static void ticketlock_lock(ticketlock* s)
{
	int x = atomic_fetch_add(&s->next, 1);
	while(x != atomic_load(&s->current)) thrd_yield();		
}
static void ticketlock_unlock(ticketlock* s) 
{ 
	 atomic_fetch_add(&s->current, 1); 
}
static void mtx_init0(mtx_t *m) { mtx_init(m, mtx_plain); }
static void spinlock_init(spinlock *l) { memset(l, 0, sizeof(*l)); }
static void ticketlock_init(ticketlock *l) { memset(l, 0, sizeof(*l)); }

#define lock_mtx_init(x) mtx_init(x, mtx_plain)
#define lock_init(x) _Generic(x, ticketlock*: ticketlock_init, spinlock*: spinlock_init, mtx_t*: mtx_init0)(x)
#define lock(x) _Generic(x, ticketlock*: ticketlock_lock, spinlock*: spinlock_lock, mtx_t*: mtx_lock)(x)
#define unlock(x) _Generic(x, ticketlock*: ticketlock_unlock, spinlock*: spinlock_unlock, mtx_t*: mtx_unlock)(x)

typedef struct PoolElement
{
	size_t size;
	void *ptr;
	char c;
} PoolElement;

struct PoolNode
{
	struct PoolNode *next;
};

/* Need thread_local. Locking is not nearly as fast as malloc. */
static thread_local size_t poolLastSize;
static mtx_t poolLock;
static thread_local struct PoolNode* poolNext;

static PoolElement* pool_alloc()
{
	union { PoolElement *e; struct PoolNode *n; char* c; } result;
//	lock(&poolLock);
	if(poolNext)
	{
		result.n = poolNext;
		poolNext = poolNext->next;
	}
	else
	{
		size_t sz = poolLastSize ? poolLastSize * 2 : 128;
		char *c = malloc(sz * sizeof(PoolElement));
		if(c)
		{
			result.c = c;
			poolNext = result.n;
			struct PoolNode *n = poolNext;
			struct PoolElement *end = result.e + sz;
			while(result.e != end)
			{			
				++result.e;
				n->next = result.n;
				n = n->next;				
			}
			n->next = 0;
			result.n = poolNext;
			poolNext = poolNext->next;
			poolLastSize = sz;
		}
		else
		{
			memset(&result, 0, sizeof(result));			
		}
	}
//	unlock(&poolLock);
	return result.e;
}
void pool_free(PoolElement *e)
{
	union { PoolElement *e; struct PoolNode *n; } x = { e };	
//	lock(&poolLock);	
	x.n->next = poolNext;
	poolNext = x.n;
//	unlock(&poolLock);
}
