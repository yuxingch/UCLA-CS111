//	pthread
int pthread_create(pthread_t *thread, NULL, void *(*start_routine) (void *), void *args);
//	remember to include -pthread

//	You can choose one of the following to exit:
pthread_exit(3);
return 3;

//	it cannot access another thread's local variable
pthread_join();
//	while loop


//	clock_gettime()
//	need to provide the start time and the end time in our program, and calculate the difference
struct timespec {
	time_t   tv_sec;        /* seconds */
	long     tv_nsec;       /* nanoseconds */
};

//	in our code, use CLOCK_MONOTONIC

//	gnuplot 5

/*	ROUTINE
 *	start_time
 *	pthread_create()
 *	in each thread
 *		for (i = 0; i < #it; i++)
 *		{add(&counter,1);}
 *		for (...)
 *		{......-1}
 *	end_time
 */

/*	CPU time and Wall time
 *		a 			b
 *	a < b? might be. unschedule
 *	a = b? possible
 *	a > b? also possible
 *	In our assignment, we use wall time.
 */


//	Why final counter value is not zero?
//		race condition, not atomic update
void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    *pointer = sum;
}
A              |          B
counter = 0    |    
sum = 0 + 1    |    sum = 0 + 1
counter = 1    |    counter = 1


QUESTION 2.1.1 - causing conflicts:
Why does it take many iterations before errors are seen?
Why does a significantly smaller number of iterations so seldom fail?

Ans: The thread function is very fast. The real problem here is pthread_create. 
		involves syscall, very expensive. If the time spent in pthread_create is alpha,
		and the time spent in for loops (+1 and -1) is beta:
		alpha < beta: at certain time, multiple threads doing the calculations, updating the counter variable.
		alpha = beta
		alpha > beta: there is no way to have race condition


//	pthread_yield and sched_yield

QUESTION 2.1.2 - cost of yielding:
Why are the --yield runs so much slower?
Where is the additional time going?
Is it possible to get valid per-operation timings if we are using the --yield option?
If so, explain how. If not, explain why not.

Ans: (1&2) We release the CPU, put into waiting queue. If CPU > threads, it will be immediately released.
			context-switch. (syscall, kernel mode)
	 (3&4) No way. 

QUESTION 2.1.3 - measurement errors:
Why does the average cost per operation drop with increasing iterations?
If the cost per iteration is a function of the number of iterations, how do we know how many iterations to run (or what the "correct" cost is)?

Ans: When we have small # of iterations, most of the time is spent on pthread_create. 
	 To see where it converges, it is hte true cost. (This is the wall time, not the CPU time.)

//------------------------

type __sync_lock_test_and_set (type *ptr, type value, ...)

int rtn = test_and_set (&lock, 1);
//	we need to check the return value of the function
//	rtn = 0:
//	rtn = 1:
//	while loop

__sync_val_compare_and_swap
//	there is no lock in compare and swap
//	don't call the original add function, write a new one
//	directly modify the counter
compare_and_swap(ptr, oldval, newvalue);
//	A function defined by ourself
void cas_add(long long * ptr, int value){
	compare_and_swap(__,  __,     __);
	            /* ptr oldval newvalue */
				/* can we use *ptr for old value?
				 * can we use *ptr+value for new value?
				 * We have to make sure oldvalue + value = newvalue
				 * (ptr, *ptr, *ptr+value) does not satisfy the previous requirement b/c another thread can modify the ptr.
				 * SOLUTION: we need to create our own varibales
				 */
}

void cas_add(long long * ptr, int value){
	long long old, new;
	old = *ptr;
	new = old + value;
	compare_and_swap(ptr, old, new);
	//	Is this correct? Not enough. We need a while loop.
}

void cas_add(long long * ptr, int value){
	long long old, new;
	old = *ptr;
	new = old + value;
	while (compare_and_swap(ptr, old, new) != old)
		;
}

cas_add(&counter, 1);
cas_add(&counter, -1);

QUESTION 2.1.4 - costs of serialization:
Why do all of the options perform similarly for low numbers of threads?
Why do the three protected operations slow down as the number of threads rises?

Ans: competetion. multiple thread competing for the locks will be very high.
	 Why spin-lock is higher? memory contention.





//------ PART 2: sorted, doubly-linked, list
//	just use spin-lock/mutex, no compare and swap

The cost first drop: averaging the cost to create threads
Then it rises: longer list.

QUESTION 2.2.1 - scalability of Mutex
Compare the variation in time per mutex-protected operation vs the number of threads in Part-1 (adds) and Part-2 (sorted lists).
Comment on the general shapes of the curves, and explain why they have this shape.
Comment on the relative rates of increase and differences in the shapes of the curves, and offer an explanation for these differences.

Ans: 

QUESTION 2.2.2 - scalability of spin locks
Compare the variation in time per protected operation vs the number of threads for list operations protected by Mutex vs Spin locks. 
Comment on the general shapes of the curves, and explain why they have this shape.
Comment on the relative rates of increase and differences in the shapes of the curves, and offer an explanation for these differences.

Ans: 



Spin lock is cheap. Mutex has queue and context-switch things. => when small # threads, spin lock lower cost.


