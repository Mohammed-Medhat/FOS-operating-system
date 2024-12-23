// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
    struct __semdata* semD=(struct __semdata*)smalloc(semaphoreName, sizeof(struct __semdata), 1);

	semD ->count=(int)value;
	semD ->lock=0;
	strncpy(semD ->name, semaphoreName, strlen(semaphoreName));
	sys_init_queue(&(semD ->queue));

	struct semaphore sem;
	sem.semdata=semD;
	return sem;
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	struct __semdata* semShared=(struct __semdata*)sget(ownerEnvID, semaphoreName);
    struct semaphore getSem;
    getSem.semdata=semShared;
    return getSem;
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
}

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
	uint32 keyw = 1;
	do {
	    keyw = xchg(&(sem.semdata ->lock),keyw);
	} while (keyw != 0);

	sem.semdata->count--;
	if (sem.semdata->count < 0) {
	/* place this process in s.queue */
	/* block this process */
		sem.semdata->lock = 0;
		sys_wait_helper(&(sem.semdata->queue));
		}
	sem.semdata->lock=0;
}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	int keys = 1;
	do {
	    keys = xchg(&(sem.semdata ->lock),keys);
	} while (keys != 0);

	sem.semdata->count++;
	if (sem.semdata->count <= 0) {
		/* remove a process P from s.queue */
		/* place process P on ready list */
		sys_signal_helper(&sem.semdata->queue);
	}
	sem.semdata->lock = 0;
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
