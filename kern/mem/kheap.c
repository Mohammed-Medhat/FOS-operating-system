#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
uint32 Frame [30000];
bool lock = 1;

//[PROJECT'24.MS2] Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
//[PROJECT'24.MS2] [USER HEAP - KERNEL SIDE] initialize_kheap_dynamic_allocator
// Write your code here, remove the panic and write your code
//panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");
if(lock){
	init_spinlock(&Klock, "kheap lock");
	lock=0;
}
uint32 Limit = (uint32)(daStart - daLimit);

uint32 virtual_address = daStart;

if(Limit >= initSizeToAllocate){
KH_START = daStart;//KERNAL_BASE 0xF6000000
KH_LIMIT = daLimit;//KERNAL_BASE+MAX
KH_BREAK = (daStart + initSizeToAllocate);//KERNAL_BASE+PAGE_SIZE

for(uint32 i = daStart; i<KH_BREAK; i+=4096){
//uint32 currentPageDir = ptr_page_directory[PDX(i)];
//
//uint32 *ptr_page_table = NULL;
//cprintf("test1: %x \n",ptr_page_table);
//get_page_table((uint32*)currentPageDir,i,&ptr_page_table);
//cprintf("test2 \n");
//if(ptr_page_table != NULL){
//uint32 currentTableEntry = ptr_page_table[PTX(i)];
//cprintf("test3 \n");
//}

uint32 *ptr_page_table = NULL;
get_page_table(ptr_page_directory,i,&ptr_page_table);

struct FrameInfo* ptrFrameInfo = get_frame_info(ptr_page_directory,i,&ptr_page_table);

allocate_frame(&ptrFrameInfo);
map_frame(ptr_page_directory,ptrFrameInfo,i,PERM_PRESENT|PERM_WRITEABLE);
Frame[to_frame_number(ptrFrameInfo)]=i;
}

initialize_dynamic_allocator(daStart,initSizeToAllocate);
return 0;
}else{
panic("Initial allocated size is too large!!!");
return -1;
}
}

void* sbrk(int numOfPages)
{
/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
 * you should allocate pages and map them into the kernel virtual address space,
 * and returns the address of the previous break (i.e. the beginning of newly mapped memory).
 * numOfPages = 0: just return the current position of the segment break
 *
 * NOTES:
 * 1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
 * or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
 */

//MS2: COMMENT THIS LINE BEFORE START CODING====
//return (void*)-1 ;
//====================================================

//[PROJECT'24.MS2] Implement this function
// Write your code here, remove the panic and write your code
//panic("sbrk() is not implemented yet...!!");

uint32 sizeToIncrease=numOfPages*PAGE_SIZE;

uint32 NEW_KH_BREAK = (uint32)(KH_BREAK+sizeToIncrease);

if(numOfPages==0)
return (void*)KH_BREAK;

if(NEW_KH_BREAK>KH_LIMIT)//OR NO MEMORY
return (void*)-1 ;

uint32 *oldBreak = (uint32*)KH_BREAK; //to return later
KH_BREAK = NEW_KH_BREAK;

for(uint32 i = (uint32)oldBreak; i<KH_BREAK; i+=PAGE_SIZE){

uint32 *ptr_page_table = NULL;
get_page_table(ptr_page_directory,i,&ptr_page_table);

struct FrameInfo* ptrFrameInfo = get_frame_info(ptr_page_directory,i,&ptr_page_table);

allocate_frame(&ptrFrameInfo);
map_frame(ptr_page_directory,ptrFrameInfo,i,PERM_PRESENT|PERM_WRITEABLE);
Frame[to_frame_number(ptrFrameInfo)]=i;
}

return oldBreak ;
}



struct MK MKs[30001];


void* kmalloc(unsigned int size)
{
    //TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
    // Write your code here, remove the panic and write your code
    //kpanic_into_prompt("kmalloc() is not implemented yet...!!");

    // use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
	 acquire_spinlock(&Klock);
     unsigned int num_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
     uint32 *pageTable;
     int counter =0;
     uint32 firstaddress=0;


     struct MK *block_info ;

     if(!isKHeapPlacementStrategyFIRSTFIT()) setKHeapPlacementStrategyFIRSTFIT();

    if (size<=DYN_ALLOC_MAX_BLOCK_SIZE){

    	 release_spinlock(&Klock);
            return alloc_block_FF(size);

    }else {

        for(uint32 i =KH_LIMIT+PAGE_SIZE ; i<KERNEL_HEAP_MAX; i+=PAGE_SIZE){

             if(get_frame_info(ptr_page_directory,i,&pageTable)==0){
                 counter++;
                 if(counter==1){
                     firstaddress=i;
                 }
                 if(counter==num_pages){
                     break;
                 }
                 if(i==KERNEL_HEAP_MAX-PAGE_SIZE){
                     firstaddress=0;
                 }
             }
             else{
                 firstaddress=0;
                 counter=0;
                 unsigned int test_size=MKs[(i-KH_LIMIT+PAGE_SIZE)/PAGE_SIZE].Ksize;
				 i+=ROUNDUP(test_size, PAGE_SIZE)-PAGE_SIZE;
				 if(i+PAGE_SIZE>=KERNEL_HEAP_MAX)break;
             }

        }
        if(firstaddress!=0){
        	int index;
        for(uint32 i =firstaddress ; i<firstaddress+(num_pages*PAGE_SIZE); i+=PAGE_SIZE){
        	//cprintf("%x \n",i);

            struct FrameInfo *frame_info = NULL;
            if(allocate_frame(&frame_info)== E_NO_MEM){
            	release_spinlock(&Klock);
                return NULL;
            }
            map_frame(ptr_page_directory,frame_info,i,PERM_PRESENT | PERM_WRITEABLE);

            uint32 framenum= to_frame_number(frame_info);

            Frame[framenum]=i;



        }
        struct MK *block_info ;
        block_info=(struct MK *)firstaddress;
        block_info->Ksize=size;
        block_info->firstAddres=firstaddress;
        index=(firstaddress-KH_LIMIT+PAGE_SIZE)/PAGE_SIZE;
        MKs[index]=*block_info;

        release_spinlock(&Klock);
        return (void*)firstaddress;

        }

        if(firstaddress!=0){
        release_spinlock(&Klock);
        return (void*)firstaddress;
            }else {
            	release_spinlock(&Klock);
                return NULL;
            }

    }
    release_spinlock(&Klock);
    return NULL;
}

void kfree(void* virtual_address) //pointer to the memory region to be freed
{
	//[PROJECT'24.MS2] Implement this function
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details
	acquire_spinlock(&Klock);
	if(!isKHeapPlacementStrategyFIRSTFIT()) setKHeapPlacementStrategyFIRSTFIT();
	if ((uint32)virtual_address<KH_BREAK && (uint32)virtual_address>=KH_START){ //block
		free_block(virtual_address);
		release_spinlock(&Klock);
		return;
	}
	else if((uint32)virtual_address>=KH_LIMIT+PAGE_SIZE && (uint32)virtual_address<KERNEL_HEAP_MAX-PAGE_SIZE){

		//print_blocks(MKlist);

		//LIST_FOREACH(page, &MKlist){
		int index=(int)((uint32)virtual_address-KH_LIMIT+PAGE_SIZE)/PAGE_SIZE;

				//LIST_REMOVE(&MKlist, page);
				unsigned int num_pages = ROUNDUP(MKs[index].Ksize, PAGE_SIZE) / PAGE_SIZE;

				for(uint32 i =(uint32)virtual_address ; i<(uint32)virtual_address+(num_pages*PAGE_SIZE); i+=PAGE_SIZE){

					uint32 *ptr_page_table;
					struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory, i, &ptr_page_table);
					free_frame(ptr_frame_info);
					unmap_frame(ptr_page_directory, i);
					uint32 framenum= to_frame_number(ptr_frame_info);
					Frame[framenum]=0;

				}
				MKs[index]=MKs[30001];
				release_spinlock(&Klock);
				return;
			}

	else panic("invalid address");


}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");
	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details
	uint32 frame_number = physical_address / PAGE_SIZE;
	uint32 offset = physical_address % PAGE_SIZE;
	uint32 virtaddr=Frame[frame_number]+offset;
	if(virtaddr<(KH_START)||(virtaddr>(KH_LIMIT)&&virtaddr<(KH_LIMIT+PAGE_SIZE)))
			return 0;

	return virtaddr;
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");
	uint32 page_offset=PGOFF(virtual_address);
	uint32 *ptr_page_table=NULL;
	get_page_table(ptr_page_directory,virtual_address,&ptr_page_table);
	if (ptr_page_table == NULL) {
	        return 0;
	    }
	struct FrameInfo *frame_info = get_frame_info(ptr_page_directory,virtual_address,&ptr_page_table);
	if (frame_info == NULL) {
	        return 0;
	    }
	uint32 physical_address = to_physical_address(frame_info) | page_offset;
	return physical_address;
	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT'24.MS2 BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code
	if(((uint32)virtual_address<KH_BREAK)&&new_size<=DYN_ALLOC_MAX_BLOCK_SIZE){
		return realloc_block_FF(virtual_address,new_size);
	}else if(((uint32)virtual_address<KH_BREAK)&&new_size>DYN_ALLOC_MAX_BLOCK_SIZE){

		void *address=kmalloc(new_size);
		if(address==NULL)return NULL;
			memcpy(address,virtual_address,get_block_size(virtual_address));
			free_block(virtual_address);
			return address;
	}else if(((uint32)virtual_address>=KH_LIMIT+PAGE_SIZE)&&new_size<=DYN_ALLOC_MAX_BLOCK_SIZE){

		void *address=alloc_block_FF(new_size);
		if(address==NULL)return NULL;
		memcpy(address,virtual_address,MKs[((uint32)virtual_address-KH_LIMIT+PAGE_SIZE)/PAGE_SIZE].Ksize);
		kfree(virtual_address);
		return address;
	}else if(((uint32)virtual_address>=KH_LIMIT+PAGE_SIZE)&&new_size>DYN_ALLOC_MAX_BLOCK_SIZE){
		int old_index=((uint32)virtual_address-KH_LIMIT+PAGE_SIZE)/PAGE_SIZE;
		uint32 old_size =MKs[old_index].Ksize;
		uint32 old_num_pages=ROUNDUP(old_size, PAGE_SIZE) / PAGE_SIZE;
		uint32 new_num_pages=ROUNDUP(new_size, PAGE_SIZE) / PAGE_SIZE;
		int flag=0;
		if(new_size==old_size){
			return virtual_address;
		}
		else if(new_size>old_size){
			int counter=0;
			uint32 diff_of_pages=new_num_pages-old_num_pages;
			uint32 *pageTable=0;
			uint32 first_i=(uint32)virtual_address+(old_num_pages*PAGE_SIZE)-PAGE_SIZE;
			for(uint32 i = first_i; i<first_i+(diff_of_pages*PAGE_SIZE); i+=PAGE_SIZE){
				if(get_frame_info(ptr_page_directory,i,&pageTable)==0){
					 counter++;
					 if(counter==diff_of_pages){
						 flag=1;
						 break;
					 }
					 if(i==KERNEL_HEAP_MAX-PAGE_SIZE){
						 break;
					 }
				 }

		}

			if(flag==1){
				for(uint32 i =first_i ; i<first_i+(diff_of_pages*PAGE_SIZE); i+=PAGE_SIZE){
					struct FrameInfo *frame_info = NULL;
					if(allocate_frame(&frame_info)== E_NO_MEM)
						return NULL;
					map_frame(ptr_page_directory,frame_info,i,PERM_PRESENT | PERM_WRITEABLE);
					uint32 framenum= to_frame_number(frame_info);
					Frame[framenum]=i;
				}
				MKs[old_index].Ksize=new_size;
				return virtual_address;
			}
			void *address=kmalloc(new_size);
			if(address==NULL)return NULL;
			memcpy(address,virtual_address,MKs[old_index].Ksize);
			MKs[old_index]=MKs[30001];
			kfree(virtual_address);
			return address;
		}else{
			uint32 diff_of_pages=old_num_pages-new_num_pages;
			uint32 *pageTable=0;
			uint32 first_i=(uint32)virtual_address+(new_num_pages*PAGE_SIZE)-PAGE_SIZE;
			for(uint32 i =first_i ; i<first_i+(diff_of_pages*PAGE_SIZE); i+=PAGE_SIZE){
				uint32 *ptr_page_table;
				struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory, i, &ptr_page_table);
				free_frame(ptr_frame_info);
				unmap_frame(ptr_page_directory, i);
				uint32 framenum= to_frame_number(ptr_frame_info);
				Frame[framenum]=0;
			}
			MKs[old_index].Ksize=new_size;
			return virtual_address;
		}
	}

	return NULL;
	//panic("krealloc() is not implemented yet...!!");
}
