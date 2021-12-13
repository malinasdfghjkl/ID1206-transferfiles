#define _GNU_SOURCE
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>
#include "green.h"

#define FALSE 0
#define TRUE 1

#define STACK_SIZE 4096

#define PERIOD 100

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, FALSE};

static green_t *running = &main_green;

struct green_t *ready_queue = NULL;

static sigset_t block;

void timer_handler(int);

static void init() __attribute__((constructor));

void init()
{
    getcontext(&main_cntx);

    // Timer initialization
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

void enqueue(green_t **list, green_t *thread)
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

green_t *dequeue(green_t **list)
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
    green_t *this = running;

    void *result = (*this->fun)(this->arg);
    // place waiting (joining) thread in ready queue
    if (this->join != NULL)
    {
        enqueue(&ready_queue, this->join);
    }
    // save result of execution
    this->retval = result;

    //free the space
    free(this->context->uc_stack.ss_sp);
    // we're a zombie
    this->zombie = TRUE;
    // find the next thread to run
    green_t *next = dequeue(&ready_queue);

    running = next;
    setcontext(next->context);
}

int green_create(green_t *new, void *(*fun)(void *), void *arg)
{
    // Set up the new context and stack for the new thread.
    ucontext_t *cntx = (ucontext_t *)malloc(sizeof(ucontext_t));
    getcontext(cntx);

    void *stack = malloc(STACK_SIZE);

    cntx->uc_stack.ss_sp = stack;
    cntx->uc_stack.ss_size = STACK_SIZE;
    makecontext(cntx, green_thread, 0);

    // Set up the new thread.
    new->context = cntx;
    new->fun = fun;
    new->arg = arg;
    new->next = NULL;
    new->join = NULL;
    new->retval = NULL;
    new->zombie = FALSE;

    // add new to the ready queue
    sigprocmask(SIG_BLOCK, &block, NULL);
    enqueue(&ready_queue, new);
    sigprocmask(SIG_UNBLOCK, &block, NULL);

    return 0;
}

int green_yield()
{
    green_t *susp = running;
    // add susp to ready queue
    enqueue(&ready_queue, susp);
    // select the next thread for execution
    green_t *next = dequeue(&ready_queue);

    running = next;
    // swap context
    swapcontext(susp->context, next->context);
    return 0;
}

int green_join(green_t *thread, void **res)
{
    sigprocmask(SIG_BLOCK, &block, NULL);
    if (!thread->zombie)
    {
        green_t *susp = running;
        // add as joining thread
        thread->join = susp;
        // select the next thread for execution
        green_t *next = dequeue(&ready_queue);
        running = next;
        swapcontext(susp->context, next->context);
    }
    // collect result
    if (thread->retval != NULL && res != NULL)
    {
        *res = thread->retval;
    }
    // free context
    free(thread->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

//      # # # # # # # # # #
//      # # # # # # # # # #
//      #  3. Conditions  #
//      # # # # # # # # # #
//      # # # # # # # # # #

// Initialize a green condition variable
void green_cond_init(green_cond_t *cond)
{
    sigprocmask(SIG_BLOCK, &block, NULL);
    cond->queue = NULL;
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

// move the first suspended variable to the ready queue
void green_cond_signal(green_cond_t *cond)
{
    sigprocmask(SIG_BLOCK, &block, NULL);
    // Do not do anything if the queue is empty
    if (cond->queue == NULL)
    {
        return;
    }
    // returning a previously suspended thread and then queueing it up to be used again.
    green_t *thread = dequeue(&cond->queue);
    enqueue(&ready_queue, thread);

    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

//      # # # # # # # # # #
//      # # # # # # # # # #
//      # # # 4. Timer  # #
//      # # # # # # # # # #
//      # # # # # # # # # #

void timer_handler(int sig)
{
    green_t *susp = running;

    // add the running to the ready queue
    enqueue(&ready_queue, susp);
    // find the next thread for execution
    green_t *next = dequeue(&ready_queue);
    running = next;
    swapcontext(susp->context, next->context);
}

//      # # # # # # # # # #
//      # # # # # # # # # #
//      # # # 5. mutex  # #
//      # # # # # # # # # #
//      # # # # # # # # # #

int green_mutex_init(green_mutex_t *mutex)
{
    sigprocmask(SIG_BLOCK, &block, NULL);

    // initialize fields
    mutex->taken = FALSE;
    mutex->suspthreads = NULL;

    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

int green_mutex_lock(green_mutex_t *mutex)
{
    // block timer interupt
    sigprocmask(SIG_BLOCK, &block, NULL);

    green_t *susp = running;

    if (mutex->taken)
    {
        // suspend the current thread
        enqueue(&mutex->suspthreads, susp);
        // find the next thread
        green_t *next = dequeue(&ready_queue);
        assert(next != NULL);

        running = next;
        swapcontext(susp->context, next->context);
    }
    else
    {
        // take the lock
        mutex->taken = TRUE;
    }
    // unblock
    sigprocmask(SIG_UNBLOCK, &block, NULL);

    return 0;
}

int green_mutex_unlock(green_mutex_t *mutex)
{
    // block timer interrupt
    sigprocmask(SIG_BLOCK, &block, NULL);

    if (mutex->suspthreads != NULL)
    {
        // move suspended thread to ready queue
        green_t *suspthreads = dequeue(&mutex->suspthreads);
        enqueue(&ready_queue, suspthreads);
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
//      # 6. final touch  #
//      # # # # # # # # # #
//      # # # # # # # # # #

void green_cond_wait(green_cond_t *cond, green_mutex_t *mutex)
{
    // block timer interrupt
    sigprocmask(SIG_BLOCK, &block, NULL);

    // suspend the running thread on condition
    green_t *susp = running;
    assert(susp != NULL);

    enqueue(&cond->queue, susp);

    if (mutex != NULL)
    {
        // release the lock if we have a mutex
        mutex->taken = FALSE;
        green_t *susp = dequeue(&mutex->suspthreads);
        // move suspended thread to the ready queue
        enqueue(&ready_queue, susp);
        mutex->suspthreads = NULL;
    }

    green_t *next = dequeue(&ready_queue);
    assert(next != NULL);

    running = next;
    swapcontext(susp->context, next->context);

    if (mutex != NULL)
    {
        // try to take the lock
        if (mutex->taken)
        {
            // Bad luck, suspend
            green_t *susp = running;
            enqueue(&mutex->suspthreads, susp);

            green_t *next = dequeue(&ready_queue);
            running = next;
            swapcontext(susp->context, next->context);
        }
        else
        {
            // Take the lock
            mutex->taken = TRUE;
        }
    }
    // Unblock
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return;
}