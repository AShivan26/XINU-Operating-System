#ifndef _LAB1_H
#define _LAB1_H

#define EXPDISTSCHED 1
#define LINUXSCHED 2

int sch;
extern void setschedclass(int sched_class);
extern int getschedclass();

#endif