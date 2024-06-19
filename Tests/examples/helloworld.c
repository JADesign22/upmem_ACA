#include <stdlib.h>
#include <stdio.h>

#include <defs.h>
#include <mutex.h>
#include <barrier.h>


int shared_var = 1;

MUTEX_INIT(my_mutex);
BARRIER_INIT(my_barrier, NR_TASKLETS);

#define BUFFERSIZE 32
__mram buffer[BUFFERSIZE]
int main(){

    mutex_lock(my_mutex);
    shared_var ++;
    mutex_unlock(my_mutex);
    barrier_wait(&my_barrier);
    printf("\n RUN ");
    if(!me()){
        printf("\n Value shared %d", shared_var);
    }

    
    return 0;
}
