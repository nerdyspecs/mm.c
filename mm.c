/* 
 * mm-handout.c -  Simple allocator based on implicit free lists and 
 *                 first fit placement (similar to lecture4.pptx). 
 *                 It does not use boundary tags and does not perform
 *                 coalescing. Thus, it tends to run out of memory 
 *                 when used to allocate objects in large traces  
 *                 due to external fragmentation.
 *
 * Each block has a header of the form:
 * 
 *      31                     3  2  1  0 
 *      -----------------------------------
 *     | s  s  s  s  ... s  s  s  0  0  a/f
 *      ----------------------------------- 
 * 
 * where s are the meaningful size bits and a/f is set 
 * iff the block is allocated. The list has the following form:
 *
 * begin                                                         end
 * heap                                                          heap  
 *  -----------------------------------------------------------------   
 * |  pad   | hdr(8:a) |   pad   | zero or more usr blks | hdr(8:a) |
 *  -----------------------------------------------------------------
 *    four  | prologue |  four   |                       | epilogue |
 *    bytes | block    |  bytes  |                       | block    |
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "mm.h"
#include "memlib.h"
#include "memhelper.h"


/* Team structure */
team_t team = {
	/* Team name */
	"ateam",
	/* note that we will add a 10% bonus for
	* working alone */
	/* the maximum number of members per team
	 * is four */
	/* First member's full name */
	"Witty Srisa-an",
	/* First member's email address */
	"witty@cse.unl.edu",
	/* Second member's full name (leave
	* blank if none) */
	"",
	/* Second member's email address
	* (leave blank if none) */
	"",
	/* Third member's full name
	* (leave blank if none) */
	"",
	/* Third member's email
	* address (leave blank
	* if none) */
	"",
	/* Fourth member's full name
	* (leave blank if none) */
	"",
	/* Fourth member's email
	* address (leave blank
	* if none) */
	""
};



/* Global variables */
static char *heap_listp;  /* pointer to first block */  

/* function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void printblock(void *bp);

/* 
 * mm_init - Initialize the memory manager 
 */
/* $begin mminit */
int mm_init(void) 
{
   /* create the initial empty heap */
   if ((heap_listp = mem_sbrk(4*WSIZE)) == NULL)
		return -1;
   PUT(heap_listp, KEY);               /* alignment padding */
   PUT(heap_listp+WSIZE, PACK(DSIZE, 0));  /* prologue header */ 
   PUT(heap_listp+DSIZE, PACK(0, 0));  /* empty word*/ 
   PUT(heap_listp+DSIZE+WSIZE, PACK(0, 0));   /* epilogue header */
   heap_listp += (DSIZE);


   /* Extend the empty heap with a free block of CHUNKSIZE bytes */
   if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
		return -1;
   return (long) heap_listp;
}
/* $end mminit */

/* 
 * mm_malloc - Allocate a block with at least size bytes of payload 
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size) 
{
    size_t asize;      /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char *bp;      
	//	printf("call mm_malloc\n");

    /* Ignore spurious requests */
    if (size <= 0)
		return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= WSIZE)
		asize = WSIZE + OVERHEAD;
    else
		asize = DSIZE * ((size + (OVERHEAD) + (DSIZE-1)) / DSIZE);
	 //printf("asize = %d\n", asize);
    
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
		place(bp, asize);
		return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
	 //printf("extendsize = %d\n", extendsize);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
	 {
	 	printf("mm_malloc = NULL\n");
		return NULL;
	 }
	 //printf("return address = %p\n", bp);
    place(bp, asize);
	 //mm_checkheap(1);
    return bp;
} 
/* $end mmmalloc */
/* 
 * mm_free - Free a block 
 */
/* $begin mmfree */
void mm_free(void *bp)
{
        char* hp = heap_listp;

        size_t size = GET_SIZE(HDRP(bp));
        PUT(HDRP(bp),PACK(size,1));

	if(GET_ALLOC(HDRP(NEXT_BLKP(bp))) == 1){
		//bp = NEXT_BLKP(bp);
                PUT(HDRP(bp),PACK((size+=GET_SIZE(HDRP(NEXT_BLKP(bp)))),1));
	}


	int i = 0;
	while(((int)NEXT_BLKP(hp) <(int)bp) && ((int)hp<(int)mem_heap_hi)){
	//	printf("%d\n", i);
	//	printf("no\n");
		hp = NEXT_BLKP(hp);
		i++;
	}	
	//	printf("yes\n");
	//	printf("%p  %p\n",hp, bp);
	//	printf("%p  %p\n",NEXT_BLKP(hp),bp);
        if(GET_ALLOC(HDRP(hp)) == 1){
                //bp = NEXT_BLKP(bp);
		PUT(HDRP(hp),PACK(size+=GET_SIZE(HDRP(hp)),1));
        //      printf("front got coalsed\n");
		//PUT(HDRP(bp),PACK((size+=GET_SIZE(HDRP(NEXT_BLKP(bp)))),1));

        }else{
		//printf("didnt do it\n");
	}
//	printf("---------------------------\n");

	/* You need to implement this function */
}


//* $end mmfree */

/*
 * mm_realloc - naive implementation of mm_realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
	void* newptr =mm_malloc(size);
	if (size == 0){
	mm_free(ptr);
//	return 0;
	}

	if (ptr == NULL){
		return mm_malloc(size);
	}
	
	
	size_t psize = GET_SIZE(HDRP(ptr));
	if(size <= psize){
//	psize = size;
	memcpy(newptr,ptr,size);
	mm_free(ptr);
	return newptr;
	}else if(size > psize){
	memcpy(newptr,ptr,psize);
	mm_free(ptr);
	return newptr;
	}
	
/* You need to implement this function. */
//	printf("pointer %p is having %d size and needs %d size\n",ptr,(GET_SIZE(HDRP(ptr))), size);
	    
}

/* 
 * mm_checkheap - Check the heap for consistency 
 */
void mm_checkheap(int verbose) 
{
	char *bp = heap_listp;

	if (verbose)
		printf("Heap (%p):\n", heap_listp);
	if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || GET_ALLOC(HDRP(heap_listp)))
		printf("Bad prologue header\n");

	for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
		if (verbose)
			printblock(bp);
	}

	if (verbose)
		printblock(bp);
	if ((GET_SIZE(HDRP(bp)) != 0) || (GET_ALLOC(HDRP(bp))))
		printf("Bad epilogue header\n");
}

/* The remaining routines are internal helper routines */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;
	
    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1) 
		return NULL;

    /* Initialize free block header and the epilogue header */
    PUT(HDRP(bp), PACK(size, 1));         /* free block header */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 0)); /* new epilogue header */
    return bp;
}
/* $end mmextendheap */
