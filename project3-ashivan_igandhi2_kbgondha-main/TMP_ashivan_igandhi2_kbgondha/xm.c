/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
	STATWORD        ps;
	disable(ps);

	if(virtpage>=4096 && (source>=0 || source<8) && (npages>0 || npages<=256))
	{
		if (bsm_map(currpid,virtpage,source,npages) != SYSERR)
		{
			restore(ps);
			return(OK);
		}
	}
	restore(ps);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
	STATWORD        ps;
	disable(ps);
	
	if(virtpage>=4096)
	{
		bsm_unmap(currpid, virtpage, 0);
		restore(ps);
		return(OK);
	}

	restore(ps);	
	return SYSERR;			
}
