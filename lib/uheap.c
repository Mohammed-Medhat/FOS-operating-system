#include <inc/lib.h>

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}
struct MoPageAllocator Userallocated[NUM_OF_UHEAP_PAGES]; //={0};

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	unsigned int num_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	int counter =0;
	uint32 firstaddress=0;
	int index;



	     if(!sys_isUHeapPlacementStrategyFIRSTFIT()){

	    	 sys_set_uheap_strategy(UHP_PLACE_FIRSTFIT);
	     }

	    if (size<=DYN_ALLOC_MAX_BLOCK_SIZE){
	        return alloc_block_FF(size);
	    }
	    else {

	        for(uint32 i =myEnv->UH_LIMIT+PAGE_SIZE ; i<USER_HEAP_MAX; i+=PAGE_SIZE){
	        	index=(i-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;

	             if(Userallocated[index].marked==0){
	                 counter++;
	                 if(counter==1){
	                     firstaddress=i;
	                 }
	                 if(counter==num_pages){

	                     break;
	                 }
	                 if(i==USER_HEAP_MAX-PAGE_SIZE){
	                     firstaddress=0;
	                 }
	             }
	             else{
	                 firstaddress=0;
	                 counter=0;
	                 unsigned int test_size=Userallocated[index].Ksize;
	                 i+=ROUNDUP(test_size, PAGE_SIZE)-PAGE_SIZE;
	                 if(i+PAGE_SIZE>=USER_HEAP_MAX)break;
	             }

	        }
	        if(firstaddress!=0){

	        	index=(firstaddress-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;


	        	sys_allocate_user_mem(firstaddress,size);


	            Userallocated[index].firstAddres=firstaddress;

	            Userallocated[index].Ksize=size;


	            for(uint32 i =firstaddress ; i<firstaddress+(num_pages*PAGE_SIZE); i+=PAGE_SIZE){
	            index=(i-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;
	        	Userallocated[index].marked=1;
	            }
	        return (void*)firstaddress;

	        }

	return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

}

}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
    //TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
    // Write your code here, remove the panic and write your code
    //panic("free() is not implemented yet...!!");
    //If virtual address inside the [BLOCK ALLOCATOR] range
    //Use dynamic allocator to free the given address

    if ((uint32)virtual_address >= myEnv->UH_START && (uint32)virtual_address < myEnv->UH_LIMIT) {
    	free_block(virtual_address);
        return;
    }
    else if((uint32)virtual_address>=myEnv->UH_LIMIT+PAGE_SIZE && (uint32)virtual_address<USER_HEAP_MAX){

    	int index=(int)((uint32)virtual_address-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;
        unsigned int num_pages=ROUNDUP(Userallocated[index].Ksize,PAGE_SIZE)/PAGE_SIZE;
        sys_free_user_mem((uint32)virtual_address,(uint32)Userallocated[index].Ksize);
        for(uint32 i =(uint32)virtual_address ; i<(uint32)virtual_address+(num_pages*PAGE_SIZE); i+=PAGE_SIZE){
        index=(int)((uint32)i-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;
        Userallocated[index].marked=0;
        Userallocated[index].firstAddres=0;
        Userallocated[index].Ksize=0;
        }

    }
    else{panic("invalid address /n");}
    }


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	unsigned int num_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	int counter =0;
	uint32 firstaddress=0;
	int index;
	     if(sys_isUHeapPlacementStrategyFIRSTFIT()){
	     for(uint32 i =myEnv->UH_LIMIT+PAGE_SIZE ; i<USER_HEAP_MAX; i+=PAGE_SIZE)
	     {
	    	 	   index=(i-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;

	    	 	   if(Userallocated[index].marked==0){
	    	 	       counter++;
	    	 	       if(counter==1){
	    	 	           firstaddress=i;
	    	 	       }
	    	           if(counter==num_pages){

 	                     break;
	    	           }
	    	 	       if(i==USER_HEAP_MAX-PAGE_SIZE){
	                      firstaddress=0;
	  	 	           }
	               }
	    	 	   else{
	                   firstaddress=0;
	                   counter=0;
	    	     }
 	        }
	        if(firstaddress!=0){

	        	index=(firstaddress-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;
	    	 	Userallocated[index].Ksize=size;
	           	Userallocated[index].firstAddres=firstaddress;

	           	int objectExistence=sys_createSharedObject(sharedVarName,size,isWritable,(void*)firstaddress);
	    	    if (objectExistence==E_SHARED_MEM_EXISTS|| objectExistence<0) return NULL;
	    	    Userallocated[index].id=objectExistence;


	    	 	for(uint32 i =firstaddress ; i<firstaddress+(num_pages*PAGE_SIZE); i+=PAGE_SIZE){
	                index=(i-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;

	    	 	    Userallocated[index].marked=1;
	    	 	 }
	    	 	 return (void*)firstaddress;
	        }
	    	return NULL;
	     }
return NULL;

}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");

	int objectSize=sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	if (objectSize==E_SHARED_MEM_NOT_EXISTS) return NULL;
	unsigned int num_pages=ROUNDUP(objectSize, PAGE_SIZE) / PAGE_SIZE;
	int counter =0;
	uint32 firstaddress=0;
	int index;
	if(sys_isUHeapPlacementStrategyFIRSTFIT()){
		for(uint32 i =myEnv->UH_LIMIT+PAGE_SIZE ; i<USER_HEAP_MAX; i+=PAGE_SIZE){
		             index=(i-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;

		             if(Userallocated[index].marked==0){
		                 counter++;
		                 if(counter==1){
		                     firstaddress=i;
		                 }
		                 if(counter==num_pages){

		                     break;
		                 }
		                 if(i==USER_HEAP_MAX-PAGE_SIZE){
		                     firstaddress=0;
		                 }
		             }
		             else{
		                 firstaddress=0;
		                 counter=0;
		             }

		}
		if(firstaddress==0) return NULL;
		index=(firstaddress-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;
		Userallocated[index].Ksize=objectSize;
		Userallocated[index].firstAddres=firstaddress;
		int objectToGet=sys_getSharedObject(ownerEnvID,sharedVarName,(void*)firstaddress);
		Userallocated[index].id=objectToGet;
		//cprintf("object ID gotten: %d\n", objectToGet);
		for(uint32 i =firstaddress ; i<firstaddress+(num_pages*PAGE_SIZE); i+=PAGE_SIZE){
		    index=(i-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;
		    Userallocated[index].marked=1;
	    }
		if(objectToGet==E_SHARED_MEM_NOT_EXISTS) return NULL;
		return (void*)firstaddress; //Expected = 82002000, Actual = 82001000 - 1 page (4KB)
		//return (void*)objectToGet; //Expected = 82001000, Actual = 78014000 - wrong
	}
	return NULL;

}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
	//panic("sfree() is not implemented yet...!!");
		int index;
		uint32 startVA=0;
		void* endVA;
		int32 objectID;
		int found=0;
		for(uint32 i =myEnv->UH_LIMIT+PAGE_SIZE ; i<USER_HEAP_MAX; i+=PAGE_SIZE){
			index=(i-myEnv->UH_LIMIT+PAGE_SIZE)/PAGE_SIZE;
			startVA=Userallocated[index].firstAddres;
			endVA = (void*)(startVA + (uint32)Userallocated[index].Ksize);
			if (virtual_address>=(void*)startVA && virtual_address<endVA){
				//cprintf("found in boundaries\n");
				objectID=Userallocated[index].id;
				found=1;
				break;
			}
		}
		//cprintf("The start address is: %p\n", (void*)startVA);
		//cprintf("The end address is: %p\n", endVA);
		//cprintf("The given virtual address is: %p\n", virtual_address);
		//cprintf("object ID gotten: %d\n", objectID);
		if (found!=0){
			//cprintf("startVA !=NULL\n");
			sys_freeSharedObject(objectID,(void*)startVA);
			//cprintf("b3d el sys call\n");
		}

		return;

}


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
