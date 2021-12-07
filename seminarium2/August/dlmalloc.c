#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#define TRUE 1
#define FALSE 0

#define HEAD            (sizeof(struct head))
#define MIN(size)       (((size) > (8)) ? (size) : (8))
#define LIMIT(size)     (MIN(0) + HEAD + size)
#define ALIGN 8
#define ARENA (64 * 1024)

#define MAGIC(memory)   ((struct head *)memory - 1)
#define HIDE(block)     (void *)((struct head *)block + 1)


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
    return (struct head *)((char *)block + block -> size + HEAD); //todo, kanske inte rätt och ta bort dessa kometarer till höger//take the current pointer, cast it to a character pointer then add the size of the block plus the size of a header.
}

struct head *before(struct head *block)
{
    return (struct head *)((char *)block - block -> bsize - HEAD); //kanske fel, todo, ta bort kod till höger//same as before but now bsize bc before size and -HEAD bc previous block is before
}

//  -   -   -   -   -   -   -   -   -   --  -       --  -  split a block
struct head *split(struct head *block, int size)
{
    //now resize block and use it, then make new block of what is left
    int rsize = block -> size - size - HEAD;          //remaining size
    block -> size = rsize;      //set size

    //new block
    struct head *splt = after(block);
    splt -> bsize = block -> size;
    splt -> bfree = TRUE;     //kanske annan
    splt -> size = size;
    splt -> free = FALSE;
    struct head *aft = after(splt);
    aft-> bsize = splt -> size;
    //känns som det kommer bli fel om inte man sätter aft -> bfree till false??
    return splt;
}

//  -   -   -   -   -   -   -   -   -   --  -       --  -   a new block
struct head *arena = NULL;
struct head *new ()
{
    if (arena != NULL)
    {
        printf("one arena already allocated \n");
        return NULL;
    }

    // using mmap, but could have used sbrk
    struct head *new = mmap(NULL, ARENA,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new == MAP_FAILED)
    {
        printf("mmap failed: error %d\n", errno);
        return NULL;
    }
    /* make room for head and dummy */
    unsigned int size = ARENA - (2 * HEAD);
    new-> bfree = FALSE;
    new-> bsize = 0;
    new-> free =TRUE;
    new-> size = size;
    struct head *sentinel = after(new);
    /* only touch the status fields */
    sentinel -> bfree =  TRUE;
    sentinel-> bsize = size;
    sentinel-> free = FALSE;
    sentinel-> size = 0;
    /* this is the only arena we have */
    arena = (struct head *)new;
    return new;
}

//  -   -   -   -   -   -   -   -   -   --  -       --  -    free list

struct head *flist; 
void detach(struct head *block)
{
    //struct head * next = NULL;
    if (block-> next != NULL){
        block -> next-> prev = block -> prev;
    }
    if( block->prev != NULL){
        block -> prev-> next = block -> next;

    }
    else{
        //no prev to update, this is  first in list
        //next should now be head of list
        flist = block -> next;
    }
}

void insert(struct head *block)
{
    block-> next = flist;
    block-> prev = NULL;
    if (flist != NULL){
        flist -> prev = block;
    }

    flist = block;
}

