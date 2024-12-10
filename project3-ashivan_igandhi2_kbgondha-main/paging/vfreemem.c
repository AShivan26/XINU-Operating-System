/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{

	STATWORD ps;   
    struct mblock *p, *q;
    unsigned top;
    if (size == 0 || (unsigned)block < 4096 * NBPG) {
        restore(ps);
        return SYSERR;
    }
	disable(ps);
	
	struct mblock *vml = proctab[currpid].vmemlist;

    size = (unsigned)roundmb(size);
    for (p = vml->mnext, q = vml; p !=  (struct mblock *) NULL  && p < block; q = p, p = p->mnext);
    if (((top = q->mlen + (unsigned)q) > (unsigned)block && q != vml) || 
        (p != NULL && (size + (unsigned)block) > (unsigned)p)) {
        restore(ps);
        return SYSERR;
    }
    if (q != vml && top == (unsigned)block) {
        q->mlen += size;  
    } else {
        block->mlen = size;
        block->mnext = p;
        q->mnext = block;
        q = block;
    }
    if ((unsigned)(q->mlen + (unsigned)q) == (unsigned)p) {
        q->mlen += p->mlen;
        q->mnext = p->mnext;
    }
    restore(ps);
    return OK;


}
