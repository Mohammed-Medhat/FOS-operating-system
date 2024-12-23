/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk), is_free_block(blk)) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================

void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2 (salah: least significant bit)
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...
	LIST_INIT(&freeBlocksList);

	int usableSize = initSizeOfAllocatedSpace - 2*sizeof(int);

	uint32 *BEGIN_BLOCK = (uint32*)daStart;
	*BEGIN_BLOCK = 1;
	uint32 *END_BLOCK = (uint32*)(daStart + usableSize + sizeof(int));
	*END_BLOCK = 1;

	uint32 *blockHeader = (uint32*)(daStart +sizeof(int));
	*blockHeader = usableSize;
	uint32 *blockFooter = (uint32*)(daStart + usableSize);
	*blockFooter = usableSize;


	LIST_INSERT_HEAD(&freeBlocksList, (struct BlockElement*)(daStart +2*sizeof(int)));
}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("set_block_data is not implemented yet");
	//Your Code is Here...
	if (va == NULL)
	        return;
	if(totalSize<4*sizeof(int)){
		totalSize=4*sizeof(int);
	}

	uint32 *Header = (uint32*)(va - sizeof(int));
	uint32 *Footer = (uint32*)(va + (totalSize- 2*sizeof(int)));

	*Header = totalSize & ~1;
	*Footer = totalSize & ~1;

	if(isAllocated){
			*Header=totalSize|1;
			*Footer=totalSize|1;
		}
}


//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
//==================================================================================
//DON'T CHANGE THESE LINES==========================================================
//==================================================================================
{
if (size % 2 != 0) size++;//ensure that the size is even (to use LSB as allocation flag)
if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
size = DYN_ALLOC_MIN_BLOCK_SIZE ;
if (!is_initialized)
{

uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
uint32 da_break = (uint32)sbrk(0);
initialize_dynamic_allocator(da_start, da_break - da_start);
}
}
//==================================================================================
//==================================================================================

//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
//COMMENT THE FOLLOWING LINE BEFORE START CODING
//panic("alloc_block_FF is not implemented yet");
//Your Code is Here...
uint32 neededSize = size + (uint32)(2*sizeof(int));

struct BlockElement* blk;

LIST_FOREACH(blk, &freeBlocksList) {
        uint32  block_size= get_block_size(blk);
        if ((uint32)neededSize <= (uint32)block_size) {
            if (block_size - neededSize >= (uint32)(4*sizeof(int))) {
                struct BlockElement* new_blk = (struct BlockElement*)((char*)blk + neededSize);
                set_block_data(new_blk, block_size - neededSize, 0);
                LIST_INSERT_AFTER(&freeBlocksList, blk, new_blk);
            } else {
                neededSize = block_size;
            }
            set_block_data(blk, neededSize, 1);
            LIST_REMOVE(&freeBlocksList, blk);
            return (void *)blk;

        }
}

uint32 *oldBreak = sbrk(1);

if(oldBreak!=(void*)-1){

uint32 *lastBlockFooter = (uint32*)((uint32)oldBreak - 2*sizeof(int));
int LSB = *lastBlockFooter & 1;
uint32 lastBlockSize = *lastBlockFooter & 0xFFFFFFFE;

uint32 *NEW_END_BLOCK = (uint32*)((uint32)oldBreak + PAGE_SIZE - sizeof(int));
*NEW_END_BLOCK = 1;

struct BlockElement* block = (struct BlockElement*)(oldBreak);
if(LSB){
	set_block_data(block, PAGE_SIZE, 0);
	LIST_INSERT_TAIL(&freeBlocksList,block);
	return alloc_block_FF(size);
}else{
	struct BlockElement* lastBlockVa = LIST_LAST(&freeBlocksList);
	LIST_REMOVE(&freeBlocksList, lastBlockVa);
	set_block_data(lastBlockVa, PAGE_SIZE+lastBlockSize, 0);
	LIST_INSERT_TAIL(&freeBlocksList,lastBlockVa);
	return alloc_block_FF(size);
}

}else{
	return NULL;
}
}//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...
	{
	if (size % 2 != 0) size++;//ensure that the size is even (to use LSB as allocation flag)
	if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
	size = DYN_ALLOC_MIN_BLOCK_SIZE ;
	if (!is_initialized)
	{
	uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
	uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
	uint32 da_break = (uint32)sbrk(0);
	initialize_dynamic_allocator(da_start, da_break - da_start);
	}
	}

	uint32 neededSize = size + (uint32)(2*sizeof(int));
	struct BlockElement* blk;
	struct BlockElement* bestFitBlock = NULL;
	uint32 minDiff = 0xFFFFFFFF;

	LIST_FOREACH(blk, &freeBlocksList) {
	    uint32 block_size = get_block_size(blk);
	    if (neededSize <= block_size) {
	        uint32 currentDiff = block_size - neededSize;
	        if (currentDiff < minDiff) {
	            minDiff = currentDiff;
	            bestFitBlock = blk;
	        }
	    }
	}

	if (bestFitBlock != NULL) {
	    uint32 block_size = get_block_size(bestFitBlock);

	    if (block_size - neededSize >= (uint32)(4*sizeof(int))) {
	        struct BlockElement* new_blk = (struct BlockElement*)((char*)bestFitBlock + neededSize);
	        set_block_data(new_blk, block_size - neededSize, 0);
	        LIST_INSERT_AFTER(&freeBlocksList, bestFitBlock, new_blk);
	    } else {
	        neededSize = block_size;
	    }

	    set_block_data(bestFitBlock, neededSize, 1);
	    LIST_REMOVE(&freeBlocksList, bestFitBlock);
	    return (void *)bestFitBlock;
	}

	return NULL;

}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va) {
//	TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
//	COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("free_block is not implemented yet");
//	Your Code is Here...

    uint32 size = get_block_size(va);

    uint32 *Header = (uint32*)((char*)va - sizeof(int));
    uint32 *Footer = (uint32*)((char*)va + size - 2 * sizeof(int));


    uint32 headerFooterData = size & 0xFFFFFFFE;
    *Header = headerFooterData;
    *Footer = headerFooterData;


    //BEFORE
    uint32 *beforeFooter = (uint32*)((char*)va - 2 * sizeof(int));
    int beforeLSB = *beforeFooter & 1;
    if (!beforeLSB) {
        uint32 prevSize = *beforeFooter & 0xFFFFFFFE;
        size += prevSize;
        Header = (uint32*)((char*)beforeFooter - prevSize + sizeof(int));
        va = (void*)(Header + 1);

        LIST_REMOVE(&freeBlocksList, (struct BlockElement*)((char*)Header + sizeof(int)));
    }

    // AFTER
    uint32 *afterHeader = (uint32*)((char*)Footer + sizeof(int));
    int afterLSB = *afterHeader & 1;
    if (!afterLSB) {
        uint32 afterSize = *afterHeader & 0xFFFFFFFE;
        size += afterSize;
        Footer = (uint32*)((char*)afterHeader + afterSize - sizeof(int));

        LIST_REMOVE(&freeBlocksList, (struct BlockElement*)((char*)afterHeader + sizeof(int)));
    }


    headerFooterData = size & 0xFFFFFFFE;
    *Header = headerFooterData;
    *Footer = headerFooterData;


    struct BlockElement* temp = (struct BlockElement*)va;

    if (LIST_EMPTY(&freeBlocksList)) {
        LIST_INSERT_HEAD(&freeBlocksList, temp);
    } else {
        struct BlockElement* blk;
        LIST_FOREACH(blk, &freeBlocksList) {
            if (temp < blk) {
                LIST_INSERT_BEFORE(&freeBlocksList, blk, temp);
                return;
            }
        }
        LIST_INSERT_TAIL(&freeBlocksList, temp);
    }
}

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size) {

	if (new_size % 2 != 0) {
        new_size++;
    }
    if (va == NULL && new_size == 0)
    	 return NULL;



    if (va == NULL)
        return alloc_block_FF(new_size);


    if (new_size == 0) {
        free_block(va);
        return NULL;
    }

    if (new_size < 2 * sizeof(uint32))
        new_size = 2 * sizeof(int);


    uint32 total_new_size = new_size + 2 * sizeof(int);
    uint32 old_size = get_block_size(va);
    uint32 *afterHeader = (uint32*)(va + old_size - sizeof(int));
    int afterLSB = *afterHeader & 1;
    uint32 *Header = (uint32*)((char*)va - sizeof(int));
    uint32 *Footer = (uint32*)((char*)va + old_size - 2 * sizeof(int));
    if (old_size > total_new_size && afterLSB) {
        if ((old_size - total_new_size) < 4 * sizeof(int)) {
            return va;
        }

        set_block_data(va, total_new_size, 1);
        struct BlockElement* temp = (struct BlockElement*)((char*)va + total_new_size);
        set_block_data(temp, old_size - total_new_size, 0);
        if (LIST_EMPTY(&freeBlocksList)) {
            LIST_INSERT_HEAD(&freeBlocksList, temp);
        } else {
            struct BlockElement* blk;
            LIST_FOREACH(blk, &freeBlocksList) {
                if (temp < blk) {
                    LIST_INSERT_BEFORE(&freeBlocksList, blk, temp);
                    return va;
                }
            }
            LIST_INSERT_TAIL(&freeBlocksList, temp);
        }

        return va;
    }

    else if (old_size > total_new_size && !afterLSB) {
        uint32 afterSize = get_block_size(afterHeader+sizeof(int));
        struct BlockElement* oldFree = (struct BlockElement*)((char*)afterHeader + sizeof(int));
        LIST_REMOVE(&freeBlocksList, oldFree);

        set_block_data(va, total_new_size, 1);

        struct BlockElement* temp = (struct BlockElement*)(va + total_new_size);
        set_block_data(temp, old_size - total_new_size + afterSize, 0);
        if (LIST_EMPTY(&freeBlocksList)) {
            LIST_INSERT_HEAD(&freeBlocksList, temp);
        } else {
            struct BlockElement* blk;
            LIST_FOREACH(blk, &freeBlocksList) {
                if (temp < blk) {
                    LIST_INSERT_BEFORE(&freeBlocksList, blk, temp);
                    return va;
                }
            }
            if (blk == NULL) {
                LIST_INSERT_TAIL(&freeBlocksList, temp);
            }
        }
        return va;
    }

    else if (old_size < total_new_size && afterLSB) {
    	void* new_va = alloc_block_FF(new_size);
		if (new_va == NULL) {
			 return va;
		}
		memcpy(new_va,va,old_size);
		free_block(va);
		return new_va;
	}


    else if (old_size < total_new_size && !afterLSB) {
        uint32 afterSize = get_block_size((void*)((char*)afterHeader + sizeof(uint32)));
        if (old_size + afterSize >= total_new_size) {

            struct BlockElement* oldFree = (struct BlockElement*)((char*)afterHeader + sizeof(uint32));
            uint32 remaining = old_size + afterSize - total_new_size;
            if (remaining < 4 * sizeof(uint32)) {
                set_block_data(va, old_size + afterSize, 1);
                LIST_REMOVE(&freeBlocksList, oldFree);
            } else {
                set_block_data(va, total_new_size, 1);
                struct BlockElement* temp = (struct BlockElement*)((char*)va + total_new_size);
                set_block_data(temp, remaining, 0);
                LIST_INSERT_BEFORE(&freeBlocksList, oldFree, temp);

                LIST_REMOVE(&freeBlocksList, oldFree);
            }
            return va;
        } else {
            void* new_va = alloc_block_FF(new_size);
            if (new_va == NULL) {
            	return va;
            }
            memcpy(new_va,va,old_size);
            free_block(va);
            return new_va;
            }
        }
    return NULL;
}
/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
