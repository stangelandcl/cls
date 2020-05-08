#include <stdatomic.h>
#include <threads.h>
#include <stdint.h>

#define atomic(x) _Atomic x

/* Faster than ticket lock. For when it is okay for the same thread to keep acquiring the lock 
 * repeatedly before giving other threads a chance */
typedef struct spinlock
{
	atomic(int32_t) current;
} spinlock;
static void spinlock_acquire(spinlock* s)
{
	int32_t actual;
	int count = 1000;
	do{
		if(atomic_compare_exchange_strong_explicit(&s->current, &actual, 1, memory_order_acquire, memory_order_acquire)) return;
	} while(--count);

	while(!atomic_compare_exchange_strong_explicit(&s->current, &actual, 1, memory_order_acquire, memory_order_acquire))
		thrd_yield();
		//;
}
static void spinlock_release(spinlock* s) 
{ 
	 atomic_store_explicit(&s->current, 0, memory_order_release);
}


/* Slower than spin lock. For when threads should be serviced in FIFO order */
typedef struct ticketlock
{
	atomic(int32_t) current;
	atomic(int32_t) next;
} ticketlock;
static void ticketlock_acquire(ticketlock* s)
{
	int32_t x = atomic_fetch_add(&s->next, 1);
	while(x != atomic_load(&s->current)) thrd_yield();		
}
static void ticketlock_release(ticketlock* s) 
{ 
	 atomic_fetch_add(&s->current, 1); 
}
