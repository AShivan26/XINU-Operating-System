/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
void setup_page_frame(unsigned long vaddr, pt_t *pte, pd_t *pde) {

	int frame, next_frame, bs_id, pageth; 

	get_frm(&frame);
	insert_queue(frame);

	pte->pt_pres = 1;
	pte->pt_write = 1;
	pte->pt_base = FRAME0 + frame;

	frm_tab[frame].fr_status = FRM_MAPPED;
	frm_tab[frame].fr_type = FR_PAGE;
	frm_tab[frame].fr_pid = currpid;
	frm_tab[frame].fr_vpno = vaddr/NBPG;  

	frm_tab[pde->pd_base - FRAME0].fr_refcnt++;

	bsm_lookup(currpid, vaddr, &bs_id, &pageth);
	read_bs((char*)((FRAME0 + frame) * NBPG), bs_id, pageth);

}

void insert_queue(int frame)
{
	STATWORD 	ps;
	disable(ps);
	
	int current, next;
	
	if(q_head == -1)
	{
		q_head = frame;
		restore(ps);
		return(OK);
	}
	
	current = q_head;
	while (q_tab[current].next != -1)
		current = q_tab[current].next;
	
	q_tab[current].next = frame;
	q_tab[frame].next = -1;
	

	restore(ps);
	return(OK);	
}

SYSCALL pfint()
{
	STATWORD        ps;
	disable(ps);

	unsigned long vaddr, pdbr;
	unsigned int pd_index, pt_index;
	int frame;	
	virt_addr_t *virtual_addr;
	pt_t *pte;
	pd_t *pde;
	
	pdbr = proctab[currpid].pdbr;	

	vaddr = read_cr2();
	virtual_addr = (virt_addr_t*)&vaddr;
	pd_index = virtual_addr->pd_offset;
    pt_index = virtual_addr->pt_offset;
		
 	pde = pdbr + pd_index * sizeof(pd_t);

	if(pde->pd_pres == 0)
	{
		get_frm(&frame);
		pte = (pt_t*)((FRAME0 + frame) * NBPG);
		int fr=0;
		while(fr < 1024)
		{
			pte[fr].pt_pres = 0;
			pte[fr].pt_write = 0;
			pte[fr].pt_user = 0;
			pte[fr].pt_pwt = 0;
			pte[fr].pt_pcd = 0;
			pte[fr].pt_acc = 0;
			pte[fr].pt_dirty = 0;
			pte[fr].pt_mbz = 0;
			pte[fr].pt_global = 0;
			pte[fr].pt_avail = 0;
			pte[fr].pt_base = 0;
			fr++;
		}

		pde->pd_pres = 1;
		pde->pd_write = 1;
		pde->pd_user = 0;
		pde->pd_pwt = 0;
		pde->pd_pcd = 0;
		pde->pd_acc = 0;
		pde->pd_mbz = 0;
		pde->pd_fmb = 0;
		pde->pd_global = 0;
		pde->pd_avail = 0;
		pde->pd_base = FRAME0 + frame;

		frm_tab[frame].fr_status = FRM_MAPPED;
		frm_tab[frame].fr_type = FR_TBL;
		frm_tab[frame].fr_pid = currpid;

	}
	//points newly allocated table	
	pte = (pt_t*) (pde->pd_base * NBPG + pt_index * sizeof(pt_t));					
	if(pte->pt_pres == 0)
		setup_page_frame(vaddr, pte, pde);
	
	write_cr3(pdbr);	
	restore(ps);
	return OK;
}


