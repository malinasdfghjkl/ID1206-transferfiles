//OM DEN KALLAS MALLOC, NO PRINTF, WILL LEAD TO RECURSIVE MEMORY LOOP AD INFINIUM
/*för rapporten, gör en förbättring av systemet
 * ex storlek (ordna) or
 * ex kanske fixa header
 * 
 * minst en (och högst?)
 * ordna fri listan
 * size of head (tex relativ position istället för absolut)
 * flera fri listor (tex olika storlek)
 * flera areor (multi threaded) låter komplicerat
 * 
 * 
 * Skriv en raport i latex standrad artikel
 * köra benchmarks och motivera vilka benchmark som valdes 
 * (tex många nya mallock om det är ornad fri lista/ flera fri lista,
 *  och många anrop om det är minskad header)
 * 
 */

#include "dlmalloc.c"
#include <stdio.h>
#include <stdlib.h>

//Determine suitable size that is an even multiple of ALIGN and not smaller than MIN
int adjust(size_t size){
    int minSize = MIN(size);

    //Check if minSize is an even multiple of ALIGN with modulo
    if(minSize % ALIGN == 0){
        return minSize;
    }
    //If it's not, calculate the difference and subtract to get a suitable size that is an even multiple
    else{
        int rest = minSize % ALIGN;

        return (minSize + ALIGN - rest);
    }
}

struct head *find(size_t size){
    //Making a copy of flist so we can modify it without consequences
    struct head *temp = flist;

    while(temp != NULL){
        if(temp->size >= size){
            detach(temp);

            if(temp->size >= LIMIT(size)){
                struct head *block = split(temp, size);
                insert(temp);

                struct head *aft = after(block);
                aft->bfree = FALSE;
                block->free = FALSE;

                return block;
            }
            else{
                temp->free = FALSE;
                struct head *aft = after(temp);
                aft->bfree = FALSE;
                return temp;
            }
        }
        else{
            temp = temp->next;
        }
    }

}

struct head *merge(struct head *block){
    struct head *aft = after(block);

    if(block->bfree == TRUE){
        //Unlink the block before
        struct head *bblock = before(block);
        detach(bblock);
        //Calculate and set the total size of the merged blocks
        bblock->size = HEAD + bblock->size + block->size;
        //Update the block after the merged blocks
        aft->bsize = bblock->size;
        //Continue with the merged block
        block = bblock;
    }

    if(aft->free == TRUE){
        //unlink the block
        detach(aft);
        //Calculate and set the total size of the merged blocks
        block->size = HEAD + aft->size + block->size;
        //Update the block after the merged blocks
        aft = after(block);
        aft->bsize = block->size;        
    }
    return block;
}

void *dalloc(size_t requested){
    if(requested <= 0){
        return NULL;
    }
    int size = adjust(requested);
    struct head *taken = find(size);
    if(taken == NULL){
        return NULL;
    }
    else{
        return HIDE(taken);
    }
}

void dfree(void *memory){
    if(memory != NULL){
        struct head *block = (struct head*) MAGIC(memory);

        //block = merge(block);
        struct head *aft = after(block);
        block->free = TRUE;
        aft->bfree = TRUE;
        
        //merge

        insert(block);
    }
    return;
}

void sanity(){
    //Check so that all previous pointers are correct
    struct head *sanity = flist;
    struct head *prev = sanity->prev;

    while(sanity != NULL && sanity->size != 0){
        //Check if block in flist are marked as free
        if(sanity->free != TRUE){
            printf("Not OK - found block that is not marked as free");
            exit(1);
        }
        if(sanity->size % ALIGN != 0){
            printf("Not OK - found block where size is not correct");
            exit(1);
        }
        if(sanity->prev != prev){
            printf("Not OK - found block with incorrect pointer");
            exit(1);    
        }

        prev = sanity;
        sanity = sanity->next;
    }
    printf("Sanity check complete");
    printf("\n");
}

void init(){
    struct head *first = new();
    insert(first);
}

int flistLength() {
    int amount = 0;
    int free = 0;
    struct head *current = flist;
    while(current != NULL && current->size != 0){
        amount++;
        free += current ->size;
        current = current->next;  
    }
    printf("bytes in free list is %d\n", free);
    return amount;
}