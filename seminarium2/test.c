#include "dlmall.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void testfunc()
{
printf("start start \n");
init();
printf("post-init \n");
sanity();
printf("post-sanity \n");
struct head *p= dalloc(32);
printf("dalloc done memory at %ud \n", &p);
dfree(p);
printf("dfree done %ud \n", &p);

}

double cpuSecond() {
   struct timeval tp;
   gettimeofday(&tp,NULL);
   return ((double)tp.tv_sec + (double)tp.tv_usec*1.e-6);
}

void benchmark1(){
    

    double timecount=cpuSecond();

    for(int i=0; i<100000000 ;i++){
        struct head *p= dalloc(32);
        dfree(p);

    }

    double timediff=(cpuSecond()-timecount);
    printf("%f \n", timediff);

}
void benchmark2(){
    int dallcount=0;
    int frecount=0;
     
    double timecount=cpuSecond();
    while ((cpuSecond()-timecount)<= 2.0){
        struct head *q= dalloc(32);
        dallcount++;
        dfree(q);
    }
    printf("%d \n", dallcount);
}

int main()
{
    testfunc();
    printf("\n Benchmark1:\n");
    for(int i=0; i<=100; i++){
    benchmark1();}
    printf("\n \n Benchmark2:\n");
    for(int i=0; i<=100; i++){
    benchmark2();}
}