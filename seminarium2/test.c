#include "dlmall.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#define SEED 912

double cpumSecond() {
   struct timeval tp;
   gettimeofday(&tp,NULL);
   return ((double)tp.tv_sec * 1000 + (double)tp.tv_usec * 1.e-3);
}

void testfunc()
{
    printf("start start \n");
    init();
    printf("post-init \n");
    sanity();
    printf("post-sanity \n");
    struct head *p = dalloc((size_t)32);
    printf("dalloc done memory at %u \n", &p);
    dfree(p);
    printf("dfree done %u \n", &p);
    //struct head *q= dalloc(32);
}

double cpuSecond()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
}
/*
void benchmark1(int size){
    //printf("started ");

    double timecount=cpuSecond();

    //for(int i=0; i<=100000000 ;i++){ //100000000
        struct head *p= dalloc(size);

      double timediff=(cpuSecond()-timecount);
    //printf("%f \n", timediff);
    //((if(i==100000000){
    int *flistsize;
    flistcount(flistsize);
    //printf("%d \n", *flistsize);}
    //int *bsize;
     //blockcount(size, bsize);
    //printf("size of blocks: %d", *bsize);  
        //printf("size %d \n", flistcount());

        dfree(p);
   // }

    

}
void benchmark2(int size){
    int dallcount=0;
    int frecount=0;
     
    double timecount=cpuSecond();
    while ((cpuSecond()-timecount)<= 2.0){
        struct head *q= dalloc(size);
        dallcount++;
        dfree(q);
    }
    //printf("%d \n", dallcount);
}

void benchmark3(int many, size_t a)
{
    printf("many: %d", many);
    printf("\nsize %d \n", a);
    //init();                             //init arena w/o seg fault?

    void *buf[1000];
    for (int i = 0; i < 1000; i++)
    {
        buf[i] = NULL; // NULL all buffer positions
    }

    for (int j=0; j < many; j++)
    {
        int ran = rand() % 1000;
		if(buf[ran] != NULL) {
			dfree(buf[ran]);
			buf[ran] = NULL;
		} else {
			int *mem;
			mem = dalloc(a);
			
			buf[ran] = mem;

			/* writing to the memory so we know it exists 
			*mem = 123;
		}

    }

    printf("post buf");
    int b = 1000;
    int arr[b];
    blockcount(arr, b);
   
    int sizeflist = flistcount();
    printf("size flist: %d \n", sizeflist);
    printf("array: ");
    for (int k = 0; k < sizeflist; k++)
    {
        printf("%u ", arr[k]);
    }

    printf("\nLength of the free list: %d\n", sizeflist);
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

int main(int argc, char *argv[])
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
    printf("Time: %llu\n", iElaps);
    return 0;
}