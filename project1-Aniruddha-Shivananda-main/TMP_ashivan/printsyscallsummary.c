#include <stdio.h>
#include <proc.h>
#include <kernel.h>

int IsTracing = 0;

const char* syscall_names[] = {
	"sys_freemem",
	"sys_chprio",
	"sys_getpid",
	"sys_getprio",
	"sys_gettime",
	"sys_kill",
	"sys_receive",
	"sys_recvclr",
	"sys_recvtim",
	"sys_resume",
	"sys_scount",
	"sys_sdelete",
	"sys_send",
	"sys_setdev",
	"sys_setnok",
	"sys_screate",
	"sys_signal",
	"sys_signaln",
	"sys_sleep",
	"sys_sleep10",
	"sys_sleep100",
	"sys_sleep1000",
	"sys_sreset",
	"sys_stacktrace",
	"sys_suspend",
	"sys_unsleep",
	"sys_wait"
};

void syscallsummary_start() {
	IsTracing = 1;
}

void syscallsummary_stop() {
	IsTracing = 0;
}

void printsyscallsummary() {

	kprintf("\nvoid printsyscallsummary()\n");
	int ProcessId = 0, ProcessValid = 0, i=0;

	while(ProcessId < NPROC) {
		struct	pentry *process = &proctab[ProcessId++];

		for(i=0; i < NSYSCALLS; i++) {
			if(process->freq[i] == 0)
				continue;
			else if(ProcessValid == 0) {
				kprintf("\nProcess [pid:%d]\n", ProcessId);
				ProcessValid = 1;
			}
			kprintf("Syscall: %s, count: %d, average execution time: %d (ms)\n",
			 syscall_names[i], process->freq[i], process->exec_time[i] / process->freq[i]);
		}
		ProcessValid = 0;
	}
	kprintf("\n");
}

void update_sys_call_summ(int pid, int syscall, long time_taken) {
	if(!IsTracing)
		return;
	struct pentry *process = &proctab[pid];
	process->exec_time[syscall] += time_taken;
	process->freq[syscall]++;
}