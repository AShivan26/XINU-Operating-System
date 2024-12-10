#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab1.h>

void setschedclass(int sched_class)
{
    sch = sched_class;
}

int getschedclass()
{
    return sch;
}