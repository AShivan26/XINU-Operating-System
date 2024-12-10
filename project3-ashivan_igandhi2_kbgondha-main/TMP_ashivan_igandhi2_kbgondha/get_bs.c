#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  STATWORD 	ps;
  disable(ps);
  
  if(bs_id>=0 || bs_id<8 || npages>0 || npages<=256)
  {
    if(bsm_tab[bs_id].bs_status == BSM_MAPPED){
      restore(ps);
      return bsm_tab[bs_id].bs_npages;
    }
    else{
      bsm_tab[bs_id].bs_status = BSM_MAPPED;
      bsm_tab[bs_id].bs_pid = currpid;  
      restore(ps);
      return npages;
    }
  }
  restore(ps);
  return SYSERR;
}