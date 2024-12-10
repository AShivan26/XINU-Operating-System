/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{

    STATWORD 	ps;
	disable(ps);
	
	int bs_id=0;
    //Initialize the values
    while(bs_id<8){
        bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
        bsm_tab[bs_id].bs_npages = 0;
        bsm_tab[bs_id].bs_pid = -1;
        bsm_tab[bs_id].bs_sem = 0;
        bsm_tab[bs_id].bs_vpno = 4096;
        bsm_tab[bs_id].bs_heap_shared = 0;

		bs_id++;
    }
    restore(ps);
    return OK;

}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD ps;
    disable(ps);
	//find slot
    if(avail!=NULL){
        int bs_id = 0;
        while(bs_id<8){
            if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED){
                *avail = bs_id;
                restore(ps);
                return OK;
            }
            bs_id++;
        }
    }
    restore(ps);
    return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{

    STATWORD 	ps;
	disable(ps);
	//check is mapped?
	if ((i >= 0 || i < 8) && bsm_tab[i].bs_status == BSM_MAPPED) 
	{
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = 4096; 
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_sem = 0;
		bsm_tab[i].bs_heap_shared = 0;
		
		restore(ps);		
		return(OK);
	}	

	restore(ps);	
	return SYSERR;

}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{

    STATWORD 	ps;
	disable(ps);
	
	int i=0;
	while(i<8){
		if(bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_pid == pid){
            *store = i;
            *pageth = ((vaddr/NBPG) - (bsm_tab[i].bs_vpno));
            restore (ps);
            return OK;
        }
		i++;
	}
	
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{

    STATWORD 	ps;
	disable(ps);

	if(source>=0 || source<8 || npages>0 || npages<=256){
		
		if ((bsm_tab[source].bs_heap_shared == 1 && bsm_tab[source].bs_pid == pid) || (bsm_tab[source].bs_heap_shared == 0))
		{
			proctab[pid].vhpno = vpno;
			proctab[pid].store = source;
			bsm_tab[source].bs_status = BSM_MAPPED;
			bsm_tab[source].bs_pid = pid;
			bsm_tab[source].bs_vpno = vpno;
			bsm_tab[source].bs_npages = npages;
			restore(ps);
			return(OK);
		}

	}
	restore(ps);
	return SYSERR;	

}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{

    STATWORD 	ps;
	disable(ps);
	
	//get virtual address
    long virt_addr = vpno * NBPG;
    
    int bs_id, page_offset;
 
	if (bsm_lookup(pid, virt_addr, &bs_id, &page_offset) == SYSERR) {
        restore(ps);
        return SYSERR;
    }

	int frame_index =0;
	while(frame_index<NFRAMES)
	{
		if (frm_tab[frame_index].fr_status == FRM_MAPPED && frm_tab[frame_index].fr_pid == pid && frm_tab[frame_index].fr_type == FR_PAGE)
		{
			write_bs((frame_index + NFRAMES) * NBPG, bs_id, page_offset );
			break;	
		}		
		frame_index++;		
	}

	free_bsm(bs_id);

	restore(ps);
	return(OK);
}

