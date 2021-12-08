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

typedef struct green_mutex_t {
volatile int taken;
// handle the list
        struct greeen_t *suspthreads;
} green_mutex_t;

int green_create(green_t *thread, void *(*fun)(void*), void *arg);
int green_yield();
int green_join(green_t *thread, void **val);

//conditional

void green_cond_init(green_cond_t* cond);
void green_cond_wait(green_cond_t *cond, green_mutex_t *mutex);
void green_cond_signal(green_cond_t *cond);

//timer
void timer_handler(int);

// Mutex
int green_mutex_init ( green_mutex_t *mutex );
int green_mutex_lock ( green_mutex_t *mutex );
int green_mutex_unlock ( green_mutex_t *mutex );