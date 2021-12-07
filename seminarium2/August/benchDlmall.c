#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
//#include "rand.h"
#include "dlmall.h"
//#include "dlmalloc.c"


int main(){

    //ta in argument för
    //hur många tester/loop < sen alltid samma >
    int rounds = 10000;

    int max = 20;//1024;
    int min = 8; //max/2;
    
    int blocks = 100; //antal blocks
    //int changes = 20; //antal gånger vi läser och förändrar

    //srand(time(NULL));

    void *data[blocks];
    for(int i = 0; i < blocks; i++){
        data[i] = NULL;
    }

    init();
    
    for(int i = 0; i < rounds; i++){

        //slumpa mellan 0 och block
        int index = rand() % blocks;
        int size = (rand() % (max - min)) + min; 

        //printf("%d\n", index);
        
        if(data[index] != NULL){
            dfree(data[index]);
            data[index] = NULL;
        }
        else{
            data[index] = dalloc(size);
            int *mem;
            mem = data[index];
            *mem = index;
        }
    }

    int length = flistLength();

    printf("dlmall bench completed\n");
    sanity();
    printf("number of blocks in free %d", length);

}