/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
	
{
	STATWORD 	ps;
	disable(ps);

	int backing_store_id=0;
	int pid=0;
	struct mblock *bsaddr;
	pid = create(procaddr, ssize, priority, name, nargs, args);
	
	if (get_bsm(&backing_store_id) == SYSERR)
	{
		restore(ps);
		return SYSERR;
	}

	bsm_map(pid, 4096, backing_store_id,hsize);	
	bsaddr = BACKING_STORE_BASE + (backing_store_id * BACKING_STORE_UNIT_SIZE); 	
	bsaddr->mlen = hsize * NBPG;
	bsaddr->mnext = NULL;
	bsm_tab[backing_store_id].bs_heap_shared = 1;	

	proctab[pid].vhpnpages = hsize;
	proctab[pid].vmemlist->mnext = 4096 * NBPG;
	
	restore(ps);	
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
