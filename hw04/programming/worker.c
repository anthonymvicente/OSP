#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "scheduler.h"


/*******************************************************************************
 *
 * Implement these functions.
 *
 ******************************************************************************/

/* Handler for SIGTERM signal */
void cancel_thread()
{ 
	printf("Thread %u: terminating.\n", (unsigned int)pthread_self());

	/* signal that done in queue */
	sem_post(&queue_sem);

	pthread_exit(NULL);
} 

/* TODO: Handle the SIGUSR1 signal */
sigset_t block_set;
void suspend_thread()
{   
	printf("Thread %u: suspending.\n", (unsigned int)pthread_self());

	/*add your code here to wait for a resume signal from the scheduler*/
        //sigset_t wait_set;

        /*
        sigemptyset(&wait_set);
        sigaddset(&wait_set, SIGUSR2);
        */
        int sig_received;

        sigwait(&block_set, &sig_received);
	
	printf("Thread %u: resuming.\n",(unsigned int) pthread_self());
}

/*******************************************************************************
 *
 * 
 *
 ******************************************************************************/
/*
 * waits to gain access to the scheduler queue.
 */
static int enter_scheduler_queue(thread_info_t *info)
{
	/*
	 * wait for available room in queue.
	 * create a new list entry for this thread
	 * store this thread info in the new entry.
	 */
	sem_wait(&queue_sem);
	list_elem *item = (list_elem*)malloc(sizeof(list_elem));
	info->le = item;
	item->info = info;
	item->prev = 0;
	item->next = 0;
	list_insert_tail(&sched_queue, item);
	return 0;
}

/*
 * leaves the scheduler queue
 */
void leave_scheduler_queue(thread_info_t *info)
{
	printf("Thread %lu: leaving scheduler queue.\n", info->thrid);
	/*
	 * remove the given worker from queue
	 * clean up the memory that we malloc'd for the list
	 * clean up the memory that was passed to us
	 */
	list_remove(&sched_queue, info->le);
	free(info->le);
	free(info);
}


/*
 * Initialize thread, enter scheduling queue, and execute instructions.
 * arg is a pointer to thread_info_t
 */
sigset_t unblock_set;
void *start_worker(void *arg)
{
	thread_info_t *info = (thread_info_t *) arg;
	float calc = 0.8;
	int j = 0;

        sigemptyset(&block_set);
	/* TODO: Block SIGALRM and SIGUSR2. */
        sigaddset(&block_set, SIGALRM);
        sigaddset(&block_set, SIGUSR2);
        pthread_sigmask(SIG_BLOCK, &block_set, NULL);
	 
	/* TODO: Unblock SIGUSR1 and SIGTERM. */

        sigemptyset(&unblock_set);
        sigaddset(&unblock_set, SIGUSR1);
        sigaddset(&unblock_set, SIGTERM);
        pthread_sigmask(SIG_UNBLOCK, &unblock_set, NULL);

	/* compete with other threads to enter queue. */
	if (enter_scheduler_queue(info)) {
		printf("Thread %lu: failure entering scheduler queue - %s\n", info->thrid, strerror(errno));
		free (info);
		pthread_exit(0);
	}
	printf("Thread %lu: in scheduler queue.\n", info->thrid);

	suspend_thread();

	while (1) {
		/* do some meaningless work... */
		for (j = 0; j < 10000000; j++) {
			calc = 4.0 * calc * (1.0 - calc);
		}
	}
}
