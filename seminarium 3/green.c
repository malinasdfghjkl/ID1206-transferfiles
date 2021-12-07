#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include "green.h"

#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096


static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx , NULL, NULL, NULL, NULL, FALSE} ;
static green_t *running = &main_green;

struct green_t *readyq = NULL;
static sigset_t block;

static void init() __attribute__ ( (constructor) ) ;

void init() {
getcontext(&main_cntx);
}

int green_create(green_t *new, void *(*fun)(void*), void *arg)
{
    ucontext_t *cntx = (ucontext_t *)malloc(sizeof(ucontext_t));
    getcontext(cntx);

    void *stack = malloc(STACK_SIZE);

    cntx->uc_stack.ss_sp = stack;
    cntx->uc_stack.ss_size = STACK_SIZE;
    makecontext(cntx, green_thread, 0);

    new->context = cntx;
    new->fun = fun;
    new->arg = arg;
    new->next = NULL;
    new->join = NULL;
    new->retval = NULL;
    new->zombie = NULL;

    //add new to the ready queue
    sigprocmask(SIG_BLOCK, &block, NULL);
    add_to_ready_queue(new);
    sigprocmask(SIG_UNBLOCK, &block, NULL);

    return 0;
}

void green_thread(){
green_t *this = running;

void *result = (*this->fun)(this->arg) ;
// place waiting (joining) thread in ready queue
 if(this->join != NULL){
 addreadyq(&readyq, *this->join);
 }
// save result of execution
this->retval=result;

// we're a zombie
this->zombie=TRUE;

// find the next thread to run
green_t *next= removereadyq(&readyq);
running = next;
setcontext(next->context);}