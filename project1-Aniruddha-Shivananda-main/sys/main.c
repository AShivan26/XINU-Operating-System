#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

int prX;
void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
prch(c)
char c;
{
	int i;
	sleep(5);	
}
// Process functions
void proc5(){
	sleep(1);
}
void proc4(){
	sleep(1);
}
void proc3(){
	sleep(1);
}
void proc2(){
	sleep(1);
}
void proc1(){
	sleep(1);
}

int main()
{
	//Task 1
	long param = 0xaabbccdd;
	kprintf("\nlong zfunction(long param)\n");
	long output = zfunction(param);
	kprintf("Input: 0x%08x, Output: 0x%08x\n", param, output);
	kprintf("\n");

	//Task 2
	// Create processes with different priorities
	int pr1 = create(proc1, 1024, 20, "proc1", 1, 'A');
	int pr2 = create(proc2, 4096, 15, "proc2", 1, 'B');
	int pr3 = create(proc3, 8192, 10, "proc3", 1, 'C');
	int pr4 = create(proc4, 1024, 5, "proc4", 1, 'D');
	int pr5 = create(proc5, 4096, 1, "proc5", 1, 'E'); // Highest priority

	// Resume the processes
	resume(pr1);
	resume(pr2);
	resume(pr3);
	resume(pr4);
	resume(pr5);

	printf("\nQuestion 2\n");
	int test_priority = 1;
	printprocstks(test_priority);
	printprocstks(1);

	//Task 3
	syscallsummary_start();
	resume(prX = create(prch,2000,20,"proc X",1,'A'));
	sleep(1);
	sleep(2);
	sleep(3);
	sleep(4);
	syscallsummary_stop();
	printsyscallsummary();

	return 0;
}
