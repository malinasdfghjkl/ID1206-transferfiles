#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <assert.h>
#include "green.h"

#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096

#define PERIOD 100

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, FALSE};
static green_t *running = &main_green;

struct green_t *readyq = NULL;
static sigset_t block;

static void init() __attribute__((constructor));

void init()
{
    getcontext(&main_cntx);

    //Timer initialization
    sigemptyset(&block);
    sigaddset(&block, SIGVTALRM);

    struct sigaction act = {0};
    struct timeval interval;
    struct itimerval period;

    act.sa_handler = timer_handler;
    assert(sigaction(SIGVTALRM, &act, NULL) == FALSE);
    interval.tv_sec = 0;
    interval.tv_usec = PERIOD;
    period.it_interval = interval;
    period.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &period, NULL);
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

    sigprocmask(SIG_UNBLOCK, &block, NULL);
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

//      # # # # # # # # # #
//      # # # # # # # # # #
//      #  3. Conditions  #
//      # # # # # # # # # #
//      # # # # # # # # # #

//Initialize a green condition variable
void green_cond_init(green_cond_t *cond)
{
    sigprocmask(SIG_BLOCK, &block, NULL);
    cond->suspthreads = NULL;
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

void green_cond_wait(green_cond_t *cond, green_mutex_t *mutex)
{
    //block timer interrupt
    sigprocmask(SIG_BLOCK, &block, NULL);

    //suspend the running thread on condition
    green_t *susp = running;
    assert(susp != NULL);
    enq(&cond->suspthreads, susp);
    //release the lock if we have a mutex
    //move suspended thread to ready queue
    if (mutex != NULL)
    {
        mutex->taken=FALSE;
        green_t *suspend=deq(&mutex->suspthreads);
        enq(&readyq, suspend);
        mutex->suspthreads=NULL;
    }
    //shedule next thread
    green_t *next = deq(&readyq);
    assert(next != NULL);

    running = next;
    swapcontext(susp->context, next->context);

    if (mutex != NULL)
    {
        //try to take the lock
        if (mutex->taken)
        {
            //bad luck suspended
            green_t *susp = running;
            enq(&cond->suspthreads, susp);

            green_t *next = deq(&readyq);
            running = next;
            swapcontext(susp->context, next->context);
        }
        else
        {
            //take the lock

            mutex->taken = TRUE;
        }
    }
    //unblock
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

void green_cond_signal(green_cond_t *cond)
{
    sigprocmask(SIG_BLOCK, &block, NULL);

    if (cond->suspthreads == NULL)
    {
        return;
    }

    green_t *thread = deq(&cond->suspthreads);
    enq(&readyq, thread);

    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

//      # # # # # # # # # #
//      # # # # # # # # # #
//      #  4. Timer # # # #
//      # # # # # # # # # #
//      # # # # # # # # # #

void timer_handler(int sig)
{
    green_t *susp = running;

    //add the running to the ready queue
    enq(&readyq, susp);
    //find the next thread for execution
    green_t *next = deq(&readyq);
    running = next;
    swapcontext(susp->context, next->context);
}

//      # # # # # # # # # #
//      # # # # # # # # # #
//      # #   5. Mutex  # #
//      # # # # # # # # # #
//      # # # # # # # # # #

int green_mutex_init(green_mutex_t *mutex)
{
    mutex->taken = FALSE;
    // initialize fields
    mutex->suspthreads = NULL;
}

int green_mutex_lock(green_mutex_t *mutex)
{
    //block timer interupt
    sigprocmask(SIG_BLOCK, &block, NULL);

    green_t *susp = running;

    if (mutex->taken)
    {
        //suspend the current thread
        enq(&mutex->suspthreads, susp);
        //find the next thread
        green_t *next = deq(&readyq);
        assert(next != NULL);

        running = next;
        swapcontext(susp->context, next->context);
    }
    else
    {
        //take the lock
        mutex->taken = TRUE;
    }
    //unblock
    sigprocmask(SIG_UNBLOCK, &block, NULL);

    return 0;
}

int green_mutex_unlock(green_mutex_t *mutex)
{
    sigprocmask(SIG_BLOCK, &block, NULL);
    // block timer interrupt
    if (mutex->suspthreads != NULL)
    {
        // move suspended thread to ready queue
        green_t *suspthreads = deq(mutex->suspthreads);
        enq(&readyq, suspthreads);
    }
    else
    {
        // release lock aka hard reset
        mutex->taken = FALSE;
        mutex->suspthreads = NULL;
    }
    // unblock
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

//      # # # # # # # # # #
//      # # # # # # # # # #
//      #  6. Final touch #
//      # # # # # # # # # #
//      # # # # # # # # # #
