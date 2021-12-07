#include <sys/ucontext.h>

typedef struct green_t 
{
    ucontext_t *context;            //place of origin
    void *(*fun)(void*);            //function
    void *arg;                      //argument
    struct green_t *next;           //next thread
    struct green_t *join;           //thread that wants to return from func exec
    void *retval;                   //returnvalue
    int zombie;                     //denotes that this thread has exec its function
} green_t;

typedef struct green_cond_t{
        struct green_t *suspthreads;
}green_cond_t;

int green_create(green_t *thread, void *(*fun)(void*), void *arg);
int green_yield();
int green_join(green_t *thread, void **val);

//conditional

void green_cond_init(green_cond_t* cond);
void green_cond_wait(green_cond_t *cond);
void green_cond_signal(green_cond_t *cond);