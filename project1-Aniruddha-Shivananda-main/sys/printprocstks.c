#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

static unsigned long *esp;

void printprocstks(int priority)
{
    struct pentry *proc;
    unsigned long *sp;

    int i;
    for (i = 0; i < NPROC; i++)
    {
        proc = &proctab[i];
        if (proc->pstate != PRFREE && proc->pprio > priority)
        {
            kprintf("Process [%s]\n", proc->pname);
            kprintf("    pid: %d\n", i);
            kprintf("    priority: %d\n", proc->pprio);
            kprintf("    base: 0x%08X\n", proc->pbase);
            kprintf("    limit: 0x%08X\n", proc->plimit);
            kprintf("    len: %d\n", proc->pstklen);

            if (i == currpid)
            {
                asm("movl %esp, esp");
                sp = esp;
            }
            else
            {
                sp = (unsigned long *)proc->pesp;
            }
            kprintf("    pointer: 0x%08X\n", (unsigned int)sp);
        }
    }
}