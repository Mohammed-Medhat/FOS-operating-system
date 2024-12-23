#include "channel.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <inc/string.h>
#include <inc/disk.h>

//===============================
// 1) INITIALIZE THE CHANNEL:
//===============================
// initialize its lock & queue
void init_channel(struct Channel *chan, char *name)
{
	strcpy(chan->name, name);
	init_queue(&(chan->queue));
}

//===============================
// 2) SLEEP ON A GIVEN CHANNEL:
//===============================
// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
// Ref: xv6-x86 OS code
void sleep(struct Channel *chan, struct spinlock* lk)
{
	//TODO: [PROJECT'24.MS1 - #10] [4] LOCKS - sleep
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("sleep is not implemented yet");
	//Your Code is Here...

	//sketch:
	//check if current process acquires the lock, if yes, release it, if not continue
	//release_spinlock(&(lk));
	//get_cpu_proc()->env_id //get the current process
	//el process hn3mlha block: state=ENV_BLOCKED
	//acquire spinlock
	//enqueue(chan.queue,process); //process queue //critical section
	//release spinlock
	//sched(); //context switch to the next ready process
	//let the current process to reacquire the lock acquire_spinlock(&(lk));

	struct Env *current_process= get_cpu_proc();
	//if (lk->locked && (struct cpu *)lk->cpu==(struct cpu *)get_cpu_proc()) { //casting for comparison
	//    release_spinlock(lk);  //releasing lock to avoid deadlock
	//}
	if (holding_spinlock(lk)==1){release_spinlock(lk);}//releasing lock to avoid deadlock
	current_process->env_status = ENV_BLOCKED;
   acquire_spinlock(&ProcessQueues.qlock);//(protection using spinlock)
	enqueue(&chan->queue, current_process);//critical section
	sched();  //context switching P1->P2
   release_spinlock(&ProcessQueues.qlock);
 acquire_spinlock(lk);

}

//==================================================
// 3) WAKEUP ONE BLOCKED PROCESS ON A GIVEN CHANNEL:
//==================================================
// Wake up ONE process sleeping on chan.
// The qlock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes
void wakeup_one(struct Channel *chan)
{
	//TODO: [PROJECT'24.MS1 - #11] [4] LOCKS - wakeup_one
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wakeup_one is not implemented yet");
	//Your Code is Here...

	//access the struct Env_Queue queue; in the given channel then dequeue only 1 blocked process
	//from this queue and change its state to ready (ENV_READY), then enqueue it in the ready queue by using
	//the function void sched_insert_ready0(struct Env* p); which is given

    acquire_spinlock(&ProcessQueues.qlock);//(protection using spinlock)
    if(queue_size(&(chan->queue))!=0){
    	 struct Env* process_to_wake = dequeue(&chan->queue); //critical section
    	    if (process_to_wake != NULL) {
    	    //process_to_wake->env_status = ENV_READY; //because it is already happening inside sched_insert
    	    sched_insert_ready0(process_to_wake);}//changes process status to ready and insert it in the ready queue
    	    else return; //blocked processes queue is empty, nth to wake up
    }

     release_spinlock(&ProcessQueues.qlock);
}

//====================================================
// 4) WAKEUP ALL BLOCKED PROCESSES ON A GIVEN CHANNEL:
//====================================================
// Wake up all processes sleeping on chan.
// The queues lock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes
void wakeup_all(struct Channel *chan)
{
	//TODO: [PROJECT'24.MS1 - #12] [4] LOCKS - wakeup_all
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wakeup_all is not implemented yet");
	//Your Code is Here...

	struct Env* process_to_wake;
	acquire_spinlock(&ProcessQueues.qlock);//(protection using spinlock)
	while(queue_size(&(chan->queue))!=0){ //critical section
		process_to_wake = dequeue(&(chan->queue));
		if (process_to_wake != NULL) {
		//process_to_wake->env_status = ENV_READY; //because it is already happening inside sched_insert
		sched_insert_ready0(process_to_wake);}}//changes process status to ready and insert it in the ready queue
	 release_spinlock(&ProcessQueues.qlock);
}
