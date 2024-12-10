#include <stdio.h>
#include <proc.h>
#include <kernel.h>


//Task 1
extern long zfunction(long param);

//Task 2
extern void printsegaddress();

//Task 3
extern void printtos();

//Task 4
extern void printprocstks(int priority);

//Task 5
// Assign a number from 0-26 for each system call
#define FREEMEM			0
#define CHPRIO			1
#define GETPID			2
#define GETPRIO			3
#define GETTIME			4
#define KILL			5
#define RECEIVE			6
#define RECVCLR			7
#define RECVTIM			8
#define RESUME			9
#define SCOUNT			10
#define SDELETE			11
#define SEND			12
#define SETDEV			13
#define SETNOK			14
#define SCREATE			15
#define SIGNAL			16
#define SIGNALN			17
#define SLEEP			18
#define SLEEP10			19
#define SLEEP100		20
#define SLEEP1000		21
#define SRESET			22
#define STACKTRACE		23
#define SUSPEND			24
#define UNSLEEP			25
#define WAIT			26


extern unsigned long	ctr1000;
extern int IsTracing;
extern void syscallsummary_start();
extern void syscallsummary_stop();
extern void printsyscallsummary();
extern void update_sys_call_summ(int, int, long);