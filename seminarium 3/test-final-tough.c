#include "green.h"
#include <stdio.h>

int test(int loop, green_mutex_t *mutex, green_cond_t *cond, int flag, int id)
{
while(loop > 0)
{
    green_mutex_lock(&mutex);
    while (flag != id)
    {
        green_mutex_unlock(&mutex);
        green_cond_wait(&cond, &mutex);
        green_mutex_lock(&mutex);
    }
    flag= (id + 1) % 2;
    green_cond_signal(&cond);
    green_mutex_unlock(&mutex);
    loop--;
}
}
int main (){
    
}