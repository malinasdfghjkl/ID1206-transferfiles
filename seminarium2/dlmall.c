#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

#define HEAD (sizeof(struct head))

#define MIN(size) (((size)>(8))?(size):(8))

#define LIMIT(size) (MIN(0) + HEAD + size)

#define MAGIC(memory) ((struct head*)memory -1)
#define HIDE(block) (void*)((struct head*)block +1)

#define ALIGN 8

#define ARENA (64*1024)

//  -   -   -   -   -   -   -   -   -   --  -       --  -  operations on a block
struct head
{
    uint16_t bfree;    // 2 bytes, the status of block before
    uint16_t bsize;    // 2 bytes, the size of block before
    uint16_t free;     // 2 bytes, the status of the block
    uint16_t size;     // 2 bytes, the size (max 2^16 i.e. 64 Ki byte )
    struct head *next; // 8 bytespointer
    struct head *prev; // 8 bytespointer
};

//  -   -   -   -   -   -   -   -   -   --  -       --  -  before and after
struct head *after(struct head *block)
{
    return (struct head*)((char*)block + (block->size) + HEAD); //take the current pointer, cast it to a character pointer then add the size of the block plus the size of a header.
}

struct head *before(struct head *block)
{
    return (struct head*)((char*)block + (block->bsize) - HEAD); //same as before but now bsize bc before size and -HEAD bc previous block is before
}

//  -   -   -   -   -   -   -   -   -   --  -       --  -  split a block
struct head *split(struct head *block, int size)
{
    int rsize = (block->size) - size -HEAD; //remove requested size from block size        ******HEAD
    block->size = rsize;              //set the block size to new size after split

    struct head *splt = after(block);
    splt->bsize = rsize;         //size of previous block
    splt->bfree = (block->free); //status of previous block
    splt->size = size;           //current size of block
    splt->free = FALSE;          //block has been split => no longer free

    struct head *aft = after(splt);
    aft->bsize = size;

    return splt;
}

//  -   -   -   -   -   -   -   -   -   --  -       --  -   a new block
struct head *arena = NULL;

struct head *new()
{
      //printf("in new arena: %d \n", arena);
    if(arena != NULL){
        printf("one arena already allocated \n");
        return NULL;
    }
  //printf("checked for arenas");
    // using mmap, but could have used sbrk
    struct head *new = mmap(NULL, ARENA,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
 // printf("made new struct");

    if (new == MAP_FAILED)
    {
        printf("mmap failed: error %d \n", errno);
        return NULL;
    }

  //printf("checked if map failed");
    /* make room for head and dummy */
    unsigned int size = ARENA -(2*HEAD);
    new->bfree = FALSE;                 //so we cant merge it w something else
    new->bsize = 0;                     //nthing extisted before
    new->free = TRUE;                   //allows for the block to be allocated, but hasnt been yet
    new->size = size;                   //size= the new given
    struct head *sentinel = after(new); //sets where this block ends
  //printf("set new inso and started sentinel");
    /* only touch the status fields */
    sentinel->bfree = (new->free); //status of new as new is "before"
    sentinel->bsize = (new->size); //size of new as new is "before"
    sentinel->free = FALSE;        //bc no mergeing allowed, has to bee kept so we dont lose where the block ends
    sentinel->size = 0;            //set to 0 bc not meant to contain anything, only to denote end
                                   /* this is the only arena we have */
      //printf("finished sentinel setup");
    arena = (struct head*)new;    //arena is where we may allocate memory => new
   
    return new;
}

//  -   -   -   -   -   -   -   -   -   --  -       --  -    free list

struct head *flist;
void detach(struct head *block)
{
    if (block->next != NULL)
    {
        block->next->prev = block->prev; //keeps list intact when removing block
    }
    
    if (block->prev != NULL)
    {
        block->prev->next = block->next; //same as above but in the other direction
    }
    else
    {
        flist = block->next;
    }
}

void insert(struct head *block)
{
    block->next = flist;                //********** tog boort flist->next
    block->prev = NULL;
    if (flist != NULL)
        flist->prev = block;
    flist = block;
}

//  -   -   -   -   -   -   -   -   -   --  -       --  -     -   -   -   -   -   -   -   -   -   --  -       --  -    Allocate & free

struct head *find(size_t size)
{
    struct head *sort = flist;
    while (sort)
    {
        if (sort->size >= size)
        {
            detach(sort);

            if (sort->size >= LIMIT(size))                                      //****** from > till >=
            {
                struct head *spl = split(sort, size);
                insert(sort);

                struct head *aspl = after(spl);
                aspl->bfree = FALSE;
                spl->free = FALSE;
                return spl;
            }
            else{
            struct head *asort = after(sort);
            asort->bfree = FALSE;
            sort->free = FALSE;
            return (sort);}
        }
        else
        {
            sort = sort->next;
        }
    }

    return NULL;
}

int adjust(size_t req)
{
    if (MIN(req)%ALIGN == 0)
        return MIN(req); //checks if minimum size is divisible by ALIGN
    else
    {
        return (MIN(req) + ALIGN -(MIN(req)%ALIGN));
    } //returns a small size divisble by ALIGN
}

void *dalloc(size_t request)
{
    if (request <= 0)
    {
        return NULL;
    }

    int size = adjust(request);
    struct head *taken = find(size);
    if (taken == NULL)
        return NULL;
    else
        return HIDE(taken); //don't want to include HEAD in the return
}

void dfree(void *memory)
{
    if (memory != NULL)
    {
        struct head *block = (struct head*)MAGIC(memory);
        struct head *aft = after(block); //want to adjust free statES IN THIS BLOCK AND AFTER
        block->free = TRUE;     //block is now free
        aft->bfree = TRUE;      //block before is not taken
        insert(block); //insert free block
    }
    return;
}

//  -   -   -   -   -   -   -   -   -   -   -   -   -   Merge
struct head *merge ( struct head *block) {
    struct head *aft= after(block);
    if(block->bfree) {
        struct head *bblock=before(block);
        detach(bblock);                      /* unlink the block before*/
        
        bblock->size= HEAD +(bblock->size) +(block->size);                      /* calculate and set the total size of the merged blocks*/
        aft->bsize= (bblock->size);                                                /* update the block after the merged blocks*/
        block=bblock;                                                            /* continue with the merged block */
    }

    if(aft->free){
    detach(aft);                                    /* unlink the block*/
    block->size= HEAD +(aft->size) +(block->size);    /* calculate and set the total size of merged blocks */
    aft=after(block);/* update the block after the merged block*/
    aft->bsize=(block->size);
    }

    return block;
}


// sanity

void sanity(){
    //printf("sanity pre stucts: \n");
    struct head *a = flist;

    //printf("post a %ud \n", &a);

    struct head *b = a->prev;                           //stuck here

    //printf("post b %d \n", b);

//printf("sanity: \n");

    while ( (a->size)!=0 && (a->next)!= NULL)
    {
        if ((a->size)%ALIGN != 0)
        {
            printf("Align is wrong");
            exit(1);
        }
    
        if ((a->prev) != b)
        {
            printf("previous is wrong");
            exit(1);
        }

         if (a->free != TRUE)
        {
            printf("free is wrong");
            exit(1);
        }

        if ((a->size <MIN(a->size)))
        {
            printf("min is wrong");
            exit(1);
        }

        b=a;
        a=a->next;
    }

//printf("Done \n");
}

//segmentation fix

void init(){
    
    struct head *start= new();
    insert(start);
}