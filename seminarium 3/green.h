#include <sys/ucontext.h>
#include <stdint.h>
#include <stdio.h>
#define SYSCTL_CORE_COUNT "machdep.cpu.core_count"
#include <Kernel/thread_policy.h>
#include <pthread.h>

typedef struct green_t
{
        ucontext_t *context;  //place of origin
        void *(*fun)(void *); //function
        void *arg;            //argument
        struct green_t *next; //next thread
        struct green_t *join; //thread that wants to return from func exec
        void *retval;         //returnvalue
        int zombie;           //denotes that this thread has exec its function
} green_t;

typedef struct green_cond_t
{
        struct green_t *suspthreads;
} green_cond_t;

typedef struct green_mutex_t
{
        volatile int taken;
        // handle the list
        struct green_t *suspthreads;
} green_mutex_t;

// linux specific func  implementations etc
typedef struct cpu_set
{
        uint32_t count;
} cpu_set_t;

// typedef pthread_t thread_port_t;
// typedef integer_t *thread_policy_t;

static inline void
CPU_ZERO(cpu_set_t *cs) { cs->count = 0; }

static inline void
CPU_SET(int num, cpu_set_t *cs) { cs->count |= (1 << num); }

sysctlbyname(const char *name, void *oldp, size_t *oldlenp,
             void *newp, size_t newlen);

int sched_getaffinity(pid_t pid, size_t cpu_size, cpu_set_t *cpu_set)
{
        int32_t core_count = 0;
        size_t len = sizeof(core_count);
        int ret = sysctlbyname(SYSCTL_CORE_COUNT, &core_count, &len, 0, 0);
        if (ret)
        {
                printf("error while get core count %d\n", ret);
                return -1;
        }
        cpu_set->count = 0;
        for (int i = 0; i < core_count; i++)
        {
                cpu_set->count |= (1 << i);
        }

        return 0;
}

int pthread_setaffinity_np(pthread_t thread, size_t cpu_size,
                           cpu_set_t *cpu_set)
{
  thread_port_t mach_thread;
  int core = 0;

  for (core = 0; core < 8 * cpu_size; core++) {
    if (CPU_ISSET(core, cpu_set)) break;
  }
  printf("binding to core %d\n", core);
  thread_affinity_policy_data_t policy = { core };
  mach_thread = pthread_mach_thread_np(thread);
  thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY,
                    (thread_policy_t)&policy, 1);
  return 0;
}

//end linux comp

int green_create(green_t *thread, void *(*fun)(void *), void *arg);
int green_yield();
int green_join(green_t *thread, void **val);

//conditional

void green_cond_init(green_cond_t *cond);
void green_cond_wait(green_cond_t *cond, green_mutex_t *mutex);
void green_cond_signal(green_cond_t *cond);

//timer
void timer_handler(int);

// Mutex
int green_mutex_init(green_mutex_t *mutex);
int green_mutex_lock(green_mutex_t *mutex);
int green_mutex_unlock(green_mutex_t *mutex);