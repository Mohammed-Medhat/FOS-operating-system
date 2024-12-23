#include "inc/types.h"
#include "inc/x86.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/environment_definitions.h"
#include "inc/assert.h"
#include "inc/string.h"
#include "sleeplock.h"
#include "channel.h"
#include "../cpu/cpu.h"
#include "../proc/user_environment.h"
//Lock (Spinlock): the thread continuously checks (busy waits) for the lock, consuming CPU cycles
//Sleep Lock: the thread goes to sleep and doesn�t consume CPU until the lock is released

//sleep lock initialization
//pointer lk to sleeplock structure
//string representing the name of the sleep lock, used for debugging or identification purposes
void init_sleeplock(struct sleeplock *lk, char *name)
{
	//initialization of the channel used by the sleep lock
	//A channel is often used as a mechanism to put threads to sleep and wake them up (wait queues)
	//where threads can block (sleep) until the lock is available
	init_channel(&(lk->chan), "sleep lock channel");//pointer to the chan in the structure
	//and the " " is a descriptive string

	//This initializes a spinlock that protects the internal state of the sleep lock
	//This spinlock ensures mutual exclusion when modifying the fields of the sleeplock structure
	//(e.g., the locked status and pid)
	init_spinlock(&(lk->lk), "lock of sleep lock");

	//This copies the provided name (passed as an argument) into the name field of the sleeplock structure
	strcpy(lk->name, name);
	lk->locked = 0;//the lock is currently unlocked (i.e., no thread holds the lock at the start)
	lk->pid = 0;//no process or thread is currently holding the lock
}
//function checks if the current process/thread is holding the sleep lock
int holding_sleeplock(struct sleeplock *lk)//the lock that needs to be checked
{
	int r;
	acquire_spinlock(&(lk->lk));
	//2nd part: whether the process that holds the lock (stored in lk->pid) is the same as the current process
	//the get retrieves the process ID of the current running thread
	r = lk->locked && (lk->pid == get_cpu_proc()->env_id); //critical section, if both true so the lock is held by that process
	release_spinlock(&(lk->lk));
	return r;
}
//==========================================================================

void acquire_sleeplock(struct sleeplock *lk)
{
	//TODO: [PROJECT'24.MS1 - #13] [4] LOCKS - acquire_sleeplock
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("acquire_sleeplock is not implemented yet");
	//Your Code is Here...

	//steps:
	//if condition to check if the lock is free or not, if yes
	//mark the lock as busy, if successful: continue
	//if failed: block the process on the corresponding channel (the one in the struct sleeplock) using sleep fn

	acquire_spinlock(&lk->lk);
	while (lk->locked){sleep(&lk->chan,&lk->lk);}
	lk->locked=1; //sleep lock is acquired
	lk->pid = 1; //there is a thread holding this sleep lock
	release_spinlock(&lk->lk);
}

void release_sleeplock(struct sleeplock *lk)
{
	//TODO: [PROJECT'24.MS1 - #14] [4] LOCKS - release_sleeplock
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("release_sleeplock is not implemented yet");
	//Your Code is Here...

	//use the wakeup all fn

	acquire_spinlock(&lk->lk);
	wakeup_all(&lk->chan);
	lk->locked=0; //sleep lock is released
	lk->pid = 0; //no thread is holding the sleep lock
	release_spinlock(&lk->lk);
}
