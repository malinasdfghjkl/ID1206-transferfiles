#include <stdlib.h>
#include <unistd.h>

void *malloc(size_t size){
    if( size == 0 ){
        return NULL;
    }
    
    void *memory = sbrk(size);
    if(memory == ( void *)−1) {
        return NULL;
    } else {
        return memory ;
    }
}

void free(void *memory){
return ;
}





#include < stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define ROUNDS 10
#define LOOP 100000
int main ( ) {
void *init = sb r k ( 0 ) ;
void *current ;
printf( "The initial top of the heap i s %p . \n" , init ) ;
for ( int j = 0 ; j < ROUNDS; j++) {
for ( int i= 0 ; i < LOOP ; i++) {
size_t size= ( rand ( ) % 4000 ) + sizeof( int ) ;
int *memory ;
memory = malloc(size) ;
if(memory == NULL) {
fprintf(stderr, " malloc f a i l e d \n" ) ;
return ( 1 ) ;
}

/* writing to the memory so we know it exists */
*memory = 123;
free(memory) ;
}
current= sbrk(0);
int allocated= ( int ) ( (current − init) / 1024 );
printf( "%d\n" , j ) ;
printf( "The current top o f the heap i s %p . \n" , current) ;
printf( " i n c r e a s e d by %d Kbyte\n" , allocated) ;
}
return 0 ;
}




#include < s t d l i b . h>
#include <math . h>
#define MAX 4000
#define MIN 8
int request(){
/* k i s l o g (MAX/MIN) */
double k = l o g ( ( ( double ) MAX )/ MIN ) ;
/* r i s [ 0 . . k [ */
double r =((double) (rand()%(int)(k*10000)))/ 10000;
/* s i z e i s [ 0 . . MAX[ */
int size= (int) ( (double)MAX / exp (r) ) ;
return size;
}