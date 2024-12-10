/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
//  extern int debug_mode;
SYSCALL init_frm()
{
  	STATWORD 	ps;
	disable(ps);
	
	int fr=0;
	while(fr < NFRAMES)
	{

		frm_tab[fr].fr_status = FRM_UNMAPPED;
		frm_tab[fr].fr_pid = -1;
		frm_tab[fr].fr_vpno = 0;
		frm_tab[fr].fr_refcnt = 0;
		frm_tab[fr].fr_type = FR_PAGE;
		frm_tab[fr].fr_dirty = 0;	
		fr++;			
	}
	
	restore(ps);
	return(OK);
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */

int page_replacement(){
//check reference bit set or not

	STATWORD 	ps;
	disable(ps);

	unsigned long vaddr, pdbr;
	unsigned int pd_index, pt_index;
	int current = q_head;
	int prev = -1;

	pt_t *pte;
	pd_t *pde;
	virt_addr_t *virtual_addr;
	int pg = 0;
	while(current != -1)
	{
		pg = current; 
		pdbr = proctab[currpid].pdbr;
		vaddr = frm_tab[current].fr_vpno * NBPG;  
		virtual_addr = (virt_addr_t*)&vaddr;
		pd_index = virtual_addr->pd_offset;
		pde = pdbr + pd_index * sizeof(pd_t);
		pt_index = virtual_addr->pt_offset;
		pte = (pt_t*) (pde->pd_base * NBPG + pt_index * sizeof(pt_t));
		
		if (pte->pt_acc == 1)
		{
			// kprintf("\ninside pg replacement, pt_acc is 1");
			pte->pt_acc = 0;
		}
		else
		{
			// kprintf("\ninside pg replacement, pt_acc is 0");
			// kprintf("\nassigned new page: %d", current);
			if(current==q_head)
			{
				q_head = q_tab[current].next;
			}
			else
			{
				q_tab[prev].next = q_tab[current].next;
			}

			q_tab[current].next = -1;
			restore(ps);
			if(debug_flag=1)
			{
				kprintf("\n  Frame replaced:  %d\n", pg);
			}
			return pg;	
		}
		
		prev = current;
		current = q_tab[current].next;
		// if(current == -1)
		// {
		// 	q_tab[prev].next = q_head;
		// 	current = q_head;
		// }

	}

	current=q_head;
	if(debug_flag=1)
	{
		kprintf("\n Head frame replaced:  %d\n", current);
	}
	// kprintf("\n new page outside while (head): %d", current);

	q_head=q_tab[current].next;
	q_tab[current].next = -1;
	restore(ps);
	return current;
}

SYSCALL get_frm(int* avail)
{
  	STATWORD 	ps;
	disable(ps);
	
	int i = 0;
	for (i = 0; i < NFRAMES; i++)
	{
		if (frm_tab[i].fr_status == FRM_UNMAPPED) 
		{
			*avail = i;	
			restore(ps);
			return(OK);
		}				
	}
		
	int selected_frame = -1;
	if(page_replace_policy == SC)
	{
		selected_frame = page_replacement();
		if (selected_frame > -1)
		{
			free_frm(selected_frame);
			*avail = selected_frame;	
			restore(ps);
			return(OK);
		}
	}
			
	
	restore(ps);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
    STATWORD 	ps;
	disable(ps);

	if((i == FR_TBL) && (i<FR_TBL)){
		restore(ps);
		return SYSERR;
	}

	if((i == FR_TBL) && (i<FR_DIR)){
		restore(ps);
		return SYSERR;
	}

	if(i<0 || i>NFRAMES){
		restore(ps);
		return SYSERR;
	}

	unsigned long vaddr, pdbr;
	unsigned int pd_index, pt_index;
	int bs_id, pageth;
	
	pt_t *pte;
	pd_t *pde;
	virt_addr_t *virtual_add;
	
	if(frm_tab[i].fr_type == FR_PAGE)
	{
		pdbr = proctab[frm_tab[i].fr_pid].pdbr;
		bs_id = proctab[frm_tab[i].fr_pid].store;
		pageth = frm_tab[i].fr_vpno - proctab[frm_tab[i].fr_pid].vhpno;
		vaddr = frm_tab[i].fr_vpno * NBPG;  
		virtual_add = (virt_addr_t*)&vaddr;
		pd_index = virtual_add->pd_offset;
		pt_index = virtual_add->pt_offset;
		pde = pdbr + pd_index * sizeof(pd_t);
		pte = (pt_t*) (pde->pd_base * NBPG + pt_index * sizeof(pt_t));
		int source = (i + FRAME0) * NBPG;

		write_bs(source, bs_id, pageth);
		
		int entry = pde->pd_base - FRAME0;
		frm_tab[entry].fr_refcnt--;

		if (frm_tab[entry].fr_refcnt == 0)
		{
			pde->pd_pres = 0;
			frm_tab[entry].fr_status = FRM_UNMAPPED;
			frm_tab[entry].fr_type = FR_PAGE;
			frm_tab[entry].fr_pid = -1;
			frm_tab[entry].fr_vpno = 4096;	
		} 	
	
		pte->pt_pres = 0;
		
		restore(ps);
		return(OK);			   					
	}		

	restore(ps);
	return SYSERR;
}