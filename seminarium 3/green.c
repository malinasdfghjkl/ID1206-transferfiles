#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include "green.h"

#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096


static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx , NULL, NULL, NULL, NULL, FALSE} ;
static green_t * running = &main_green;

static void init() __attribute__ ( (constructor) ) ;
void init() {
getcontext(&main_cntx) ;
}