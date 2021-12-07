#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <ucontext.h>
#include <setjmp.h>
#include <signal.h>
#include <assert.h>
#include "green.h"

#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, FALSE};
static green_t *running = &main_green;

struct green_t *readyq = NULL;
static sigset_t block;

static void init() __attribute__((constructor));

void init()
{
    getcontext(&main_cntx);
}

void enq(green_t **list, green_t *thread)
{
    if (*list == NULL)
    {
        *list = thread;
    }
    else
    {
        green_t *susp = *list;
        while (susp->next != NULL)
        {
            susp = susp->next;
        }
        susp->next = thread;
    }
}

green_t *deq(green_t **list)
{
    if (*list == NULL)
    {
        return NULL;
    }
    else
    {
        green_t *thread = *list;
        *list = (*list)->next;
        thread->next = NULL;
        return thread;
    }
}

void green_thread()
{
    sigprocmask(SIG_BLOCK, &block, NULL);

    green_t *this = running;

    void *result = (*this->fun)(this->arg);
    // place waiting (joining) thread in ready queue
    enq(&readyq, this->join);
    // save result of execution
    this->retval = result;

    // we're a zombie
    this->zombie = TRUE;

    // find the next thread to run
    green_t *next = deq(&readyq);
    running = next;
    setcontext(next->context);

    sigprocmask(SIG_BLOCK, &block, NULL);
}

int green_create(green_t *new, void *(*fun)(void *), void *arg)
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
    new->zombie = FALSE;

    //add new to the ready queue
    sigprocmask(SIG_BLOCK, &block, NULL);
    enq(&readyq, new);
    sigprocmask(SIG_UNBLOCK, &block, NULL);

    return 0;
}

int green_yield()
{
    green_t *susp = running;
    // add susp to ready queue
    enq(&readyq, susp);

    //select the next thread for execution
    green_t *next = deq(&readyq);
    running = next;
    swapcontext(susp->context, next->context);
    return 0;
}

int green_join(green_t *thread, void **res)
{
    if (!thread->zombie)
    {
        green_t *susp = running;
        // add as joining thread
        thread->join = susp;
        // select the next thread for execution
        green_t *next = deq(&readyq);
        running = next;
        swapcontext(susp->context, next->context);
    }
    // collect result
    if (thread->retval != NULL)
    {
        *res = thread->retval;
    }
    // free context
    free(thread->context);
    return 0;
}
