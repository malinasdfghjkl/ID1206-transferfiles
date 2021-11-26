#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

#define HEAD(sizeof(struct head))

#define MIN(size) (((size)>(8))?(size):(8))

#define LIMIT(size) (MIN(0)+ HEAD + size )

#define MAGIC(memory) (( struct head*)memory − 1)
#define HIDE(block) (void*) ((struct head* )block + 1)

#define ALIGN 8

#define ARENA (64*1024)


//  -   -   -   -   -   -   -   -   -   --  -       --  -  operations on a block
struct head {
uint16_t bfree; // 2 bytes, the status of block before
uint16_t bsize; // 2 bytes, the size of block before
uint16_t free; // 2 bytes, the status of the block
uint16_t size; // 2 bytes, the size (max 2^16 i.e. 64 Ki byte )
struct head *next ; // 8 bytespointer
struct head *prev ; // 8 bytespointer
};

//  -   -   -   -   -   -   -   -   -   --  -       --  -  before and after
struct head *after( struct head *block) {
return (struct head*) ((char*)block->block+size+HEAD); //take the current pointer, cast it to a character pointer then add the size of the block plus the size of a header.
}

struct head *before( struct head * block) {
return (struct head * ) ((char*)block->block+bsize-HEAD); //same as before but now bsize bc before size and -HEAD bc previous block is before
}

//  -   -   -   -   -   -   -   -   -   --  -       --  -  split a block
struct head * split( struct head * block , int size ) {
int rsize= . . . . .
block−>size= . . .
struct head * splt= . . .
splt −>b size= . . .
splt −>b free= . . .
splt −>size= . . .
splt −>free= . . .
struct head * aft= . .
aft−>b size= . . .
return splt;
}

//  -   -   -   -   -   -   -   -   -   --  -       --  -   a new block
struct head * a rena = NULL;
struct head *new ( ) {
if( a rena != NULL) {
printf( "one arena already allocated \n" ) ;
return NULL;
}

// using mmap, but could have used sbrk
struct head *new = mmap(NULL, ARENA,
PROT_READ | PROT_WRITE,
MAP_PRIVATE | MAP_ANONYMOUS, −1, 0 ) ;
i f ( new == MAP_FAILED) {
printf( "mmap failed: error %d\n" , errno ) ;
return NULL;
}
/* make room for head and dummy */
uintsize= ARENA − 2*HEAD;
new−>bfree= . . .
new−>bsize= . . .
new−>free= . . .
new−>size= . . .
struct head * sentinel= after( new ) ;
/* only touch the status fields */
sentinel −>bfree= . . .
sentinel−>bsize= . . .
sentinel−>free= . . .
sentinel−>size= . . .
/* this is the only arena we have */
a rena = ( struct head *) new ;
return new ;
}


//  -   -   -   -   -   -   -   -   -   --  -       --  -    free list

srcut head * flist;
void detach( struct head * block ) {
if( block−>next != NULL)
:
if( block−>prev != NULL)
:
else
:
}
void insert( struct head * block ) {
block−>next = . . .
block−>prev = . . .
if( flist != NULL)
:
flist= . . .
}

//  -   -   -   -   -   -   -   -   -   --  -       --  -     -   -   -   -   -   -   -   -   -   --  -       --  -    Allocate & free

void * dalloc( size_t request) {
if(request<= 0 ){
return . . .
}
int size= adjust(request);
struct head * taken = find(size);
if( taken == NULL)
return NULL;
else
return . . . .
}

void dfree( void *memory ) {
if(memory != NULL) {
struct head * block= . . .
struct head * aft= . . .
block−>free= . . .
aft−>bfree= . . .
:
}
return ;
}