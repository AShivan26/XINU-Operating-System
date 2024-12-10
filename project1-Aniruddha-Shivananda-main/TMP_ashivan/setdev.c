/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */
SYSCALL	setdev(int pid, int dev1, int dev2)
{
	unsigned long start = ctr1000;
	short	*nxtdev;

	if (isbadpid(pid)) {
		update_sys_call_summ(currpid, SETDEV, ctr1000 - start);
		return(SYSERR);
	}
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;
	update_sys_call_summ(currpid, SETDEV, ctr1000 - start);
	return(OK);
}