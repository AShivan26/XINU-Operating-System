/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  /* sanity check ! */
  STATWORD(ps);
  disable(ps);

  if(policy == SC)
  {
    page_replace_policy = policy;
    debug_flag=1;	 
    restore(ps);
    return OK;	
  }

  debug_flag=0;
  restore(ps);
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}
