Homework 04
----------------------
Author: Anthony Vicente

Files:

* README - this document
* list.c - linked list structure source
* list.h - linked list header
* scheduler.c - source file for scheduler program
* scheduler.h - scheduler program header
* smp5_tests.c - source file for running tests or something
* testrunner.c - source file for tests
* testrunner.h - header file for tests
* worker.c - source file for worker threads
* worker.h - worker header
* Makefile - makefile to compile C binary

INSTALL:

    make

USAGE:

    ./scheduler <num_threads> <queue_size> <i_1, i_2 ... i_numofthreads>
    num_threads: the number of worker threads to run
    queue_size: the number of threads that can be in the scheduler at one time
    i_1, i_2 ...i_numofthreads: the number of quanta each worker thread runs

for testing:

        ./scheduler -test -f0 rr

DETAILS:

    scheduler is a program for demonstrating the use of the round robin scheduling algorithm and how to handle signals.

UNDERSTANDING:

    The goal was to understand signals and evaluate program performance. It was important to learn how to use the functions provided primarily using the man pages. The sigaction() function was used to specify how a thread handled a given signal by pointing to the function we wanted. pthread_kill was used to send the needed signals, and sigwait was used to wait on a specific signal instead of specifying another method to handle the waiting.

QUESTIONS:

    1). We block SIGALRM and SIGUSR2 to prevent the working threads from receiving these signals as they are meant for the scheduler thread. We then unblock SIGTERM and SIGUSR1 so the thread can receive these signals from the scheduling thread to cancel or suspend as needed.

    2). sigaction() specifies how a thread will handle incoming signals while sigwait() will block on the call until the specified signal is received.

    3). Using the TIMER_ABSTIME makes the timer an absolute timer, meaning the values set in the timespec are absolute times when the timer will fire instead of times relative to when the timer_settime() function is called.

    4). When the thread received the SIGTERM signal.

    5). When all worker threads have exited and finished.

    6). The thread is removed from the head when suspended. It is removed completely when canceled.

    7). To keep track of the number of workers canceled. If our number completed is greater than or equal to our thread count, we've finished all threads.

    8). It would be easiest to modify suspend worker by not removing the worker from the head. This would keep the same worker in the head until it was complete, at which point it could move on to the next element.

    9). Use sigaction() to handle another signal. This could be useful in a program where we might want a user to be able to preempt the clock signal for one reason or another.

OUTISDE SOURCES:

    Nearly everything could be found in the man pages.
