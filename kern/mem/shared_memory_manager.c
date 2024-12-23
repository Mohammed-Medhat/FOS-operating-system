#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	acquire_spinlock(&AllShares.shareslock);
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL){
		release_spinlock(&AllShares.shareslock);
		return E_SHARED_MEM_NOT_EXISTS;
	}else{
		release_spinlock(&AllShares.shareslock);
		return ptr_share->size;
	}
	release_spinlock(&AllShares.shareslock);
	return 0;
}

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	//Your Code is Here...

	//create array of pointers with numOfFrames for frame storage, type:FrameInfo
	//this array is initialized by 0
	//if success: return pointer
	//if fail: return null

	//frames storage is in kernel heap

	 struct FrameInfo** frames_storage=(struct FrameInfo**) kmalloc(numOfFrames*sizeof(struct FrameInfo*));
	 if (frames_storage == NULL) return NULL; //allocation failed

	 for (int i = 0; i < numOfFrames; i++) frames_storage[i] = 0;

	 return frames_storage; //allocation successful
}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...

	//sketch:
	//Allocate a new shared object

	//Initialize its members:
	//references = 1,
	//ID = VA of created object after masking most significant bit to make it +ve
	//b'et el values given (parameters)

	//Create the "framesStorage": use create_frames_storage(numOfFrames)

	//Return:
	//If succeed: pointer to the created object for struct Share
	//If failed: UNDO any allocation & return NULL
	//uint32 allocation_size=ROUNDUP(size, PAGE_SIZE);//to enter page allocator
	struct Share* newsharedobject = (struct Share*)kmalloc(sizeof(struct Share));
	if (newsharedobject== NULL) return NULL; //allocation failed

	newsharedobject->references=1;
	newsharedobject->ownerID=ownerID;
    strncpy(newsharedobject->name, shareName, strlen(shareName));

    //cprintf("The start address during creation: %p\n", (void*)newsharedobject);
    //int VA=kheap_virtual_address((unsigned int) newsharedobject);
    //cprintf("shared virtual address in create share is: %p\n", newsharedobject);
    newsharedobject->ID= (int32)((int)newsharedobject & 0x7FFFFFFF); //after masking
    //cprintf("object ID in create share: %d\n", newsharedobject->ID);
    //cprintf("object ID during creation: %d\n", newsharedobject->ID);
    //cprintf("The start address during creation: %p\n", (void*)newsharedobject);

	//newsharedobject->name=shareName;
	newsharedobject->size=size;
	newsharedobject->isWritable=isWritable;
	int numOfFrames = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	newsharedobject->framesStorage=create_frames_storage(numOfFrames);
	if (newsharedobject->framesStorage== NULL) {
		kfree(newsharedobject); //undo allocation, frame storage creation failed
		return NULL;
	}
	return newsharedobject;
}


//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_share is not implemented yet");
	//Your Code is Here...

	// Iterate over the linked list of shared memory objects
    struct Share* currentShare;
	//bool lock_already_held = holding_spinlock(&AllShares.shareslock);
	//if (!lock_already_held)
	//acquire_spinlock(&AllShares.shareslock);
	//kda s7 lw held, wla a-release then reacquire?

    LIST_FOREACH(currentShare, &AllShares.shares_list){
	    if (currentShare->ownerID == ownerID &&  strncmp(currentShare->name,name,64)== 0){
	    	//release_spinlock(&AllShares.shareslock);
	    	return currentShare;
	    }
	}
   // release_spinlock(&AllShares.shareslock);
    return NULL;


}

//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...
	acquire_spinlock(&AllShares.shareslock);
	struct Env* myenv = get_cpu_proc(); //The calling environment
	//a3ml beha eh?


	struct Share* sharedObject = get_share(ownerID, shareName);
	if(sharedObject!=NULL) {
		//cprintf("alo1111\n");
		release_spinlock(&AllShares.shareslock);
		return E_SHARED_MEM_EXISTS;//already exists
	}


	else {
		sharedObject=create_share(ownerID,shareName,size,isWritable);
		if(sharedObject==NULL){
			release_spinlock(&AllShares.shareslock);
			return E_NO_SHARE;
		} //failed to create object

		//bool lock_already_held = holding_spinlock(&MemFrameLists.mfllock);

		//if (!lock_already_held)
		//{
		//	acquire_spinlock(&MemFrameLists.mfllock);
		//}

		LIST_INSERT_HEAD(&AllShares.shares_list,sharedObject);
		//cprintf("object ID in create shared shares list: %d\n", sharedObject->ID);


		unsigned int num_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
		int allocated_frames=0;
		for(uint32 i =0 ; i<num_pages; i++){
		    //cprintf("%x \n",i);
			//allocate and map, is this loop correct?
			struct FrameInfo* frame_info = NULL;

			//sharedObject->framesStorage;
		    //struct FrameInfo *frame_info = NULL;
			uint32 VA= (uint32)virtual_address+i*PAGE_SIZE;
			if(allocate_frame(&frame_info)!= 0){
				for(int j=0;i<allocated_frames;i++){
					VA=(uint32)virtual_address-i*PAGE_SIZE;
					unmap_frame(myenv->env_page_directory,VA);
				}
				release_spinlock(&AllShares.shareslock);
				return E_NO_SHARE;
			}

			uint32* ptr_page_table=NULL;
			if (get_page_table(myenv->env_page_directory,VA,&ptr_page_table) == TABLE_NOT_EXIST)
			    ptr_page_table=(uint32*)create_page_table(myenv->env_page_directory,VA);
			int mapping=map_frame(myenv->env_page_directory,frame_info,VA,PERM_WRITEABLE |PERM_USER|PERM_AVAILABLE);
		    if(mapping!=0) {
		    	release_spinlock(&AllShares.shareslock);
		    	return E_NO_SHARE;
		    }
		    sharedObject->framesStorage[i]=frame_info;
		    allocated_frames++;
		}
		release_spinlock(&AllShares.shareslock);
		return sharedObject->ID; //success
	}
	//cprintf("alo2222\n");
	release_spinlock(&AllShares.shareslock);
	return E_NO_SHARE;

}


//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	acquire_spinlock(&AllShares.shareslock);
    struct Env* myenv = get_cpu_proc();
    struct Share* sharedObject = get_share(ownerID, shareName);
    if (sharedObject == NULL) {
    	release_spinlock(&AllShares.shareslock);
    	return E_SHARED_MEM_NOT_EXISTS;
    }

    uint32 num_pages = ROUNDUP(sharedObject->size, PAGE_SIZE) / PAGE_SIZE;

    for(uint32 i =0 ; i<num_pages; i++){
    	struct FrameInfo* frame_info = NULL;
    	frame_info=sharedObject->framesStorage[i];
    	uint32 VA= (uint32)virtual_address+i*PAGE_SIZE;
    	uint32* ptr_page_table=NULL;
    	if (get_page_table(myenv->env_page_directory,VA,&ptr_page_table) == TABLE_NOT_EXIST)
    			ptr_page_table=(uint32*)create_page_table(myenv->env_page_directory,VA);
    	uint32 perm;
    	if (sharedObject->isWritable==1) perm=PERM_WRITEABLE;
        //else perm=~PERM_WRITEABLE;
    	else perm=0x000;

    	int mapping=map_frame(myenv->env_page_directory,frame_info,VA,perm |PERM_PRESENT| PERM_USER);
    	//if(mapping!=0) return E_NO_SHARE;
        sharedObject->framesStorage[i]->references++;
    }

    sharedObject->references++;
    release_spinlock(&AllShares.shareslock);
    return sharedObject->ID;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_share is not implemented yet");
	//Your Code is Here...
	//deletes the object from the shares list
		if (!holding_spinlock(&AllShares.shareslock)) acquire_spinlock(&AllShares.shareslock);
		LIST_REMOVE(&AllShares.shares_list, ptrShare);
		release_spinlock(&AllShares.shareslock);
		kfree(ptrShare->framesStorage);
		kfree(ptrShare);


}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("freeSharedObject is not implemented yet");
	//Your Code is Here...
		struct Env* myenv = get_cpu_proc();
		struct Share* currentShare;
		struct Share* object=NULL;

		LIST_FOREACH(currentShare, &AllShares.shares_list){
		    if (currentShare->ID == sharedObjectID){
		    	object=currentShare;
		    }
		}

		int numOfPages = ROUNDUP(object->size,PAGE_SIZE) / PAGE_SIZE; //=#frames
		for (uint32 i= 0; i<numOfPages; i++) {
			uint32 VA= (uint32)startVA+i*PAGE_SIZE;

				uint32 *page_table;
				unmap_frame(myenv->env_page_directory,VA);
				get_page_table(myenv->env_page_directory,VA,&page_table);
				bool isTableEmpty=0;
				if (page_table!=NULL){
					for (int j=0;j<1024;j++){
						if (page_table[j] & PERM_PRESENT){
							isTableEmpty=1;
							break;
						}
					}
					if (!isTableEmpty){
						pd_clear_page_dir_entry(myenv->env_page_directory,VA);
						kfree((void*)page_table);
					}
				}
				object->framesStorage[i]->references--;
				if(object->framesStorage[i]->references==0) {
					pt_set_page_permissions(myenv->env_page_directory, VA, 0, PERM_AVAILABLE);
					free_frame(object->framesStorage[i]);
				}
		}

		object->references--;
		if(object->references==0) free_share(object); //last share

		tlbflush();

		return 0;


}
