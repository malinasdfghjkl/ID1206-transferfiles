#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include "dlmall.h"

#define SEED 912
double cpumSecond() {
   struct timeval tp;
   gettimeofday(&tp,NULL);
   return ((double)tp.tv_sec * 1000 + (double)tp.tv_usec * 1.e-3);
}

/*void benchmark(int howMany, int requested)
{
    struct head *arr[howMany];
    srand(SEED);
    int j = 0;
    for(int i = 0; i < requested; ++i)
    {
        if(j < howMany && (i % requested * howMany) % 5 == 2)
        {
            arr[i] = dalloc(i % rand()); 
            ++j;
        }  
        else if(j > 0)
        {
            dfree(arr[i - i % rand()]);
            --j;
        }

    }

}*/

void benchmark(int howMany, int requested)
{
    printf("Hej\n");
    struct head *arr[howMany];
    srand(SEED);
    int j = 0;
    int flistSize = 0;
    for(int reset = 0; reset < howMany; ++reset)
    {
        arr[reset] = NULL;
    }
    for(int i = 0; i < requested; ++i)
    {
        int index = rand() * requested;
        if((arr[index]) != NULL)
        {
            dfree(arr[index]);
            arr[index] = NULL;
        }  
        else
        {
            arr[index] = dalloc(index);
        }
        flist_size(&flistSize);
        printf("%d\n", flistSize);
    }

}

int main(int *argc, char *argv[])
{
    init();
    // if(*argc < 3)
    // {
    //     printf("Too few arguments!\n (Format is: <Blocks requested> <Blocks at a time>");
    //     exit(1);
    // }

    int requested = atoi(argv[1]);
    int howMany = atoi(argv[2]);
    printf("%d  %d\n", requested, howMany);

    unsigned long long iStart = cpumSecond();
    benchmark(howMany, requested);
    unsigned long long iElaps = cpumSecond() - iStart;
    printf("Time: %.15llu\n", iElaps);
    return 0;
}