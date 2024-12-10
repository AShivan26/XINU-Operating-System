/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>
#include <lab1.h>
#include <stdbool.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
bool isNewEpochNeeded(){
	int i = 1;
	// Check for every process in Running and Ready queue if the value of the
	while (i < NPROC){
		if (proctab[i].pstate == PRCURR || proctab[i].pstate == PRREADY){
			if (proctab[i].counter != 0){
				return false;
			}
		}
		i++;
	}
	return true;
}

void setQuantum(){
	int i = 1;
	while (i < NPROC){
		if (proctab[i].pstate != PRFREE){
			// For a process that has never executed or used up its time quantum in the previous epoch,
			// its new quantum value is set to its process priority (i.e., quantum = priority).
			proctab[i].quantum = proctab[i].pprio;

			// For a process that has not used up its quantum in the previous epoch,
			// the scheduler allows half of the unused quantum to be carried over to the new epoch.
			if (proctab[i].counter > 0)
				if (proctab[i].counter < proctab[i].quantum)
					proctab[i].quantum = proctab[i].quantum + (proctab[i].counter / 2);
			proctab[i].counter = proctab[i].quantum;
		}
		i++;
	}
}
int resched(){
	register struct pentry *optr; /* pointer to old process entry */
	register struct pentry *nptr; /* pointer to new process entry */
	if (sch == LINUXSCHED){
		// Implement Linux Like scheduler
		optr = &proctab[currpid];
		optr->counter = preempt;
		int curr_proc_goodness_val = 0, max_goodness = 0;
		// Calculate the goodness value for the current process
		// goodness = counter + priority
		curr_proc_goodness_val = (optr->counter) ? optr->counter + optr->pprio : 0;
		int new_epoch = (isNewEpochNeeded()) ? 1 : 0;
		int i;

		if (new_epoch)
			setQuantum();

		// Find the process with max goodness value
		int which_process = NULLPROC, proc;
		proc = q[rdyhead].qnext;
		while (q[proc].qkey < MAXINT){
			int goodness = 0;
			if (proctab[proc].counter)
				goodness = proctab[proc].pprio + proctab[proc].counter;
			if (goodness > max_goodness){
				which_process = proc;
				max_goodness = goodness;
			}
			proc = q[proc].qnext;
		}
		// If the new process to be scheduled is the current running process
		// No need to context switch, just have to reset its timer and continue running the same process
		if (curr_proc_goodness_val >= max_goodness && optr->pstate == PRCURR){
			// If current process is NULLPROC preempt to Quantum
			if (currpid == NULLPROC)
				preempt = QUANTUM;
			else
				preempt = optr->counter;
			return (OK);
		}
		/* force context switch */
		if (optr->pstate == PRCURR){
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}

		// remove the identified process from redy queue and run
		nptr = &proctab[currpid = dequeue(which_process)];
		nptr->pstate = PRCURR;

		// what if process is new?
		if (which_process == NULLPROC)
			preempt = QUANTUM;
		else
			preempt = nptr->counter;
	}
	else{
		// Parameters for exponential distribution
		if (sch == EXPDISTSCHED){
			// If there is only null process and it is running then just return
			int current = q[rdyhead].qnext;
			if (((optr = &proctab[currpid])->pstate == PRCURR) &&
				(current == NULLPROC && q[current].qnext == rdytail)){
				return (OK);
			}

			// If the first process from the ready queue is running currently
			// Move it to Ready Queue
			if (optr->pstate == PRCURR){
				optr->pstate = PRREADY;
				insert(currpid, rdyhead, optr->pprio);
			}

			// generate a random number for every run
			double lambda = 0.1;
			double random_value = expdev(lambda);

			// used to store which process we want next
			// check if ready queue has any process inside
			int process_to_schedule;
			// Incase of Empty List we need to schedule NULLPROC
			if (current == rdytail){
				process_to_schedule = EMPTY;
			}
			// find the smallest number greater than the random value
			while (q[current].qkey < MAXINT && q[current].qkey <= random_value){
				current = q[current].qnext;
			}
			// If the random value is greater than highest value in ready queue
			// then pick the last process from the ready queue
			if (current == rdytail){
				process_to_schedule = q[rdytail].qprev;
			}
			// Lets say now our ready queue has processes with same priority
			// Pick the last one with same priority and schedule it
			int which_process = q[current].qnext;
			int selected_priority = q[current].qkey;
			while (q[which_process].qkey < MAXINT && q[which_process].qkey == selected_priority){
				which_process = q[which_process].qnext;
			}
			// We need to pick the one previous because now our priority is  > random_value
			process_to_schedule = q[which_process].qprev;
			// lets schedule choosen process
			currpid = (process_to_schedule == EMPTY) ? NULLPROC : dequeue(process_to_schedule);
			nptr = &proctab[currpid];
			nptr->pstate = PRCURR;
			#ifdef RTCLOCK
					preempt = QUANTUM;
			#endif
		}
		else{
			/* no switch needed if current process priority higher than next*/

			if (((optr = &proctab[currpid])->pstate == PRCURR) &&
				(lastkey(rdytail) < optr->pprio)){
				return (OK);
			}

			/* force context switch */

			if (optr->pstate == PRCURR){
				optr->pstate = PRREADY;
				insert(currpid, rdyhead, optr->pprio);
			}

			/* remove highest priority process at end of ready list */

			nptr = &proctab[(currpid = getlast(rdytail))];
			nptr->pstate = PRCURR; /* mark it currently running	*/
			#ifdef RTCLOCK
					preempt = QUANTUM; /* reset preemption counter	*/
			#endif
		}
	}
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

		/* The OLD process returns here when resumed. */
		return OK;
}
