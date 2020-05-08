#include <stdatomic.h>
#include <threads.h>

#define atomic(x) _Atomic x


/* Ticket lock. See https://concurrencyfreaks.blogspot.com/search/label/
 * ticket%20lock?updated-max=2014-12-24T10:03:00%2B01:00&max-results=20&
 * start=7&by-date=false */
typedef struct spinlock
{
	atomic(size_t) current;
	char padding[64 - sizeof(size_t)]; /* different cache line */
	atomic(size_t) next;
} spinlock
static void spinlock_acquire(spinlock* s)
{
	size_t x = atomic_fetch_add(&s->next, 1);
	while(x != atomic_load(&s->current)) thrd_yield();		
}
static void spinlock_release(spinlock* s) 
{ 
	/* atomic_fetch_add(&s->unlock, 1);  */
	size_t x = atomic_load_explicit(&self->current, memory_order_relaxed);
	atomic_store(&self->current, x + 1);
}
