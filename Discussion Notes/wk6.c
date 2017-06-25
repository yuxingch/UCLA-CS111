//	wk6
/*
QUESTION 2.3.1 - Cycles in the basic list implementation:
Where do you believe most of the cycles are spent in the 1 and 2-thread list tests?
The insert function (the list operations).
2 threads: mostly the list operations

Why do you believe these to be the most expensive parts of the code?
No thread competing for the lock.

Where do you believe most of the time/cycles are being spent in the high-thread spin-lock tests?
Where do you believe most of the time/cycles are being spent in the high-thread mutex tests?
Spinning.
Mutex: system calls. Putting into the waiting queue.

QUESTION 2.3.2 - Execution Profiling:
Where (what lines of code) are consuming most of the cycles when the spin-lock version of the list 
exerciser is run with a large number of threads?
insert
Why does this operation become so expensive with large numbers of threads?

QUESTION 2.3.3 - Mutex Wait Time:
Only one thread, wait-for-lock time is almost zero, when multiple threads, will be greater than the
avg completion time (total running time of our code)
Why higher? multiple threads:
completion time: wall time
wait-for-lock time: cpu time

Why does the average lock-wait time rise so dramatically with the number of contending threads?

				The difference: lock release/lock acquire
				v 
walltime----+++--+++----------------------->
			^
			for one list op
memory contention problem

Why does the completion time per operation rise (less dramatically) with the number of contending threads?

How is it possible for the wait time per operation to go up faster (or higher) than the completion 
time per operation?
*/

//	Addressing the Underlying Problem
A. 1key
B. 1key
C. 1key
...
hash(key) -> which sublist
//	figure out how to (safely and correctly) obtain the length of the list, which now involves 
//		enumerating all of the sub-lists.
each sublist should have a lock variable
In each of the thread, you first lock all the list, compute the list length, then release.

If two threads want to compute the list length, will there be deadlock?
for threads, you have to use the same order to get the lock to avoid deadlock.

/*
QUESTION 2.3.4 - Performance of Partitioned Lists
Explain the change in performance of the synchronized methods as a function of the number of lists.
Throughput increases slower. And it won't continue increasing when there are more and more lists.
When you have more sublists, competing the lock < list operations.

It seems reasonable to suggest the throughput of an N-way partitioned list should be equivalent to 
the throughput of a single list with fewer (1/N) threads. Does this appear to be true in the above 
curves? If not, explain why not.
Two factors determined the probability of conflict
(i) threads, competing for the lock
(ii) the number of elements in each list <=> long critical section

*/
