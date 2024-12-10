/* sleep.c - sleep */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * sleep  --  delay the calling process n seconds
 *------------------------------------------------------------------------
 */
SYSCALL	sleep(int n)
{
	unsigned long start = ctr1000;
	STATWORD ps;
	if (n<0 || clkruns==0) {
		update_sys_call_summ(currpid, SLEEP, ctr1000 - start);
		return(SYSERR);
	}
	if (n == 0) {
	        disable(ps);
		resched();
		restore(ps);
		update_sys_call_summ(currpid, SLEEP, ctr1000 - start);
		return(OK);
	}
	while (n >= 1000) {
		sleep10(10000);
		n -= 1000;
	}
	if (n > 0)
		sleep10(10*n);
	update_sys_call_summ(currpid, SLEEP, ctr1000 - start);
	return(OK);
}