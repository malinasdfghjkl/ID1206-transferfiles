#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include "dlmall.h"

unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}


double cpumSecond() {
   struct timeval tp;
   gettimeofday(&tp,NULL);
   return ((double)tp.tv_sec * 1000 + (double)tp.tv_usec * 1.e-3);
}
#define SEED 112
void benchmark(int howMany, int requested, unsigned int min, unsigned int max)
{
    //printf("Hej\n");
    struct head *arr[howMany];
    srand(SEED);
    int j = 0;
    int flistSize = 0;
    int blocknumb= 0;
    for(int reset = 0; reset < howMany; ++reset)
    {
        arr[reset] = NULL;
    }
    for(int i = 0; i < howMany; ++i)
    {
        int index = rand() % requested;
        //printf("index: %d\n", index);
        if((arr[index]) != NULL || j >= requested)
        {
            dfree(arr[index]);
            arr[index] = NULL;
            --j;
        }  
        else
        {
            arr[index] = dalloc(request(min, max));
            //printf("arr index content: %d\n", arr[index]);
            ++j;
        }
        blocknumb= flist_size(&flistSize, blocknumb);
        int blocksizes[blocknumb];
        block_sizes(blocknumb, &blocksizes);
        printf("\n %d", flistSize);
        //printf("number of blocks: %d\n", blocknumb);
        //printf("number of rounds: %d\n", i);
         //printf("block sizes: \n");
        /*for(int k=0; k<blocknumb; k++){
           printf(" %d ", blocksizes[k]);
        }*/
    }

}

int main(int argc, char *argv[])
{
    init();
    // if(*argc < 3)
    // {
    //     printf("Too few arguments!\n (Format is: <Blocks requested> <Blocks at a time>");
    //     exit(1);
    // }

    int requested = 5000;//10000;
    int howMany = 10000;//1000000;
    int min=20;
    int max= 200;
    printf("%d  %d\n", requested, howMany);
    unsigned long long iStart, iElaps;
    //for(int i = 0; i < 100000; i += 20)
    //{
        iStart = cpumSecond();
        benchmark(howMany, requested, min, max);
        iElaps = cpumSecond() - iStart;
        //printf("Time: %llu\n", iElaps);
    //}
    return 0;
}