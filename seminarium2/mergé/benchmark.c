#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <errno.h>
#include <math.h>
#include "dlmall.h"

int main(){

    //ta in argument för
    //hur många tester/loop < sen alltid samma >
    int rounds = 1000;

    int max = 20;
    int min = max/2;
    
    int blocks = 10; //antal blocks
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

    //int length = flistLength();

    printf("dlmall bench completed\n");
    //sanity();
    //printf("number of blocks in free %d", length);

}


