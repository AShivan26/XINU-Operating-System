/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
SYSCALL scount(int sem)
{
	unsigned long start = ctr1000;
	extern	struct	sentry	semaph[];

	if (isbadsem(sem) || semaph[sem].sstate==SFREE) {
		update_sys_call_summ(currpid, SCOUNT, ctr1000 - start);
		return(SYSERR);
	}
	update_sys_call_summ(currpid, SCOUNT, ctr1000 - start);
	return(semaph[sem].semcnt);
}