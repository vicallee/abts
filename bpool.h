//bpool.h by vical@126.com
#ifndef _BPOOL_H_
#define _BPOOL_H_

#include <stdlib.h> //malloc free
#include <string.h> //memset
#include <assert.h> //assert

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define inline _inline
#endif

#define SIZE_AUTO_EXPAND 0
#define DEFAULT_KERNLE_PAGESIZE 4096
#define MAX_BPOOL_INDEX 20

#define alignment_down(a, size) ((a) & (~(size-1)) )
#define alignment_up(a, size)   (((a)+size-1) & (~ (size-1)))

typedef struct bpool_t {
	unsigned int bsize; 			//The number of bytes per block
	unsigned int maxcount; 			//The maximum number of blocks that support
	unsigned int nallblock; 		//Has applied for the number of blocks
	unsigned int nfreeblock; 		//The number of free blocks
	unsigned int nbase; 			//The first number of blocks of the index[1,10]
	unsigned int nindex; 			//Has been used to index
	void **freeits[MAX_BPOOL_INDEX];//Ladder array, each ladder in two parts, the front is idle pointer table, followed by the available memory
} bpool_t;

static void bpool_init (bpool_t *bp, unsigned int maxcount, unsigned int bsize)
{
	unsigned onesize;
	memset (bp, 0, sizeof(bpool_t));
	bp->maxcount = maxcount;
	bp->bsize = alignment_up(bsize, 16);
	
	//The first, the best just one page, or no more than one page
	onesize = sizeof(void*) + bp->bsize;
	while ((unsigned)(onesize<<bp->nbase) < DEFAULT_KERNLE_PAGESIZE)
		bp->nbase++;
	if (bp->nbase > 0 && (unsigned)(onesize<<bp->nbase) > DEFAULT_KERNLE_PAGESIZE)
		bp->nbase--;
	
	if (maxcount == SIZE_AUTO_EXPAND && bp->nbase + MAX_BPOOL_INDEX > 31)
		bp->nbase = 31-MAX_BPOOL_INDEX;
}

static void bpool_cleanup (bpool_t *bp)
{
	unsigned int i;
	for (i=0; i<=bp->nindex; i++) {
		if (bp->freeits[i]) 
			free (bp->freeits[i]);
	}
}

//Calculate the number of 32bit leading 0
static inline unsigned int nlz(unsigned x)
{
   unsigned int n = 1;
   if (x == 0) return(32);
   if ((x >> 16) == 0) {n = n +16; x = x <<16;}
   if ((x >> 24) == 0) {n = n + 8; x = x << 8;}
   if ((x >> 28) == 0) {n = n + 4; x = x << 4;}
   if ((x >> 30) == 0) {n = n + 2; x = x << 2;}
   n = n - (x >> 31);
   return n;
}

// 1 2 4 8 16...2^19
static inline void *bpool_locat_block (bpool_t *bp, void *head[], unsigned int offset, unsigned int isize)
{
    //The i-th ladder have base * (i ^ 2) entries
    unsigned int x = 32 - nlz((offset>>bp->nbase) + 1) - 1;
    //Offset in the ladder
    unsigned int y = offset - (((1<<x)-1) << bp->nbase);
    char *arr = (char*)head[x];
    assert (x < MAX_BPOOL_INDEX);
    assert (y < (unsigned)((1<<x)<<bp->nbase));
    return (void*)(arr+y*isize);
}

static void *bpool_alloc_block(bpool_t *bp)
{
    unsigned int ncount = 0;
	void *node, **pret=NULL;

	if (bp->nfreeblock > 0)
	    goto __found;

	ncount = (1<<bp->nindex)<<bp->nbase;
	if (bp->maxcount!=SIZE_AUTO_EXPAND && ncount > bp->maxcount - bp->nallblock)
		ncount = bp->maxcount-bp->nallblock;
		
    if (ncount<=0 || bp->nindex == MAX_BPOOL_INDEX)        
        return NULL;

	bp->freeits[bp->nindex] = (void**)malloc ((sizeof(void*) + bp->bsize) * ncount);
	if (!bp->freeits[bp->nindex]) 
		return NULL;

	//Only need to initialize freeits
	memset (bp->freeits[bp->nindex], 0, sizeof(void*)*ncount);
	bp->nindex++;

	//There itemcount freeits undetermined their true positions of free memory.it is NULL
	//There is a ladder behind the same itemcount any contiguous memory have not been articulated, then apply again when dynamically articulated
	bp->nallblock += ncount;
	bp->nfreeblock += ncount;

__found:
    bp->nfreeblock--;
    pret = (void**)bpool_locat_block (bp, (void**)(void*)bp->freeits, bp->nfreeblock, sizeof(void*));
    if (!*pret) {
        //The original location of this ladder is not attached to the back of the memory of the ladder, this time to hook up
		ncount = (1<<(bp->nindex-1))<<bp->nbase;
		*pret = (void*)((char*)(bp->freeits[bp->nindex-1] + ncount)+bp->nfreeblock*bp->bsize);
	}

	//To take this position after setting is empty, otherwise it will contaminate repeatedly extended
    node = *pret;
    *pret = NULL;
    return node;
}

static inline void *bpool_calloc_block(bpool_t *bp)
{
	void *mem = bpool_alloc_block (bp);
	if (mem) 
		memset (mem, 0, bp->bsize);
	return mem;
}

static void bpool_free_block (bpool_t *bp, void *p)
{
	//Direct this void * attached to the last available node freeits, so after many malloc or free, freeits where pointer pointing across the ladder
	void **pret = (void**)bpool_locat_block (bp, (void**)(void*)bp->freeits, bp->nfreeblock, sizeof(void*));
	assert (pret && bp->nfreeblock < bp->nallblock);
	*pret = p;
	bp->nfreeblock++;
}

#ifdef __cplusplus
}
#endif

#endif //_BPOOL_H_
