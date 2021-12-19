#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <time.h>
#include "SSU_Sem.h"

#define NUM_THREADS 3
#define NUM_ITER 10

SSU_Sem sem;
SSU_Sem par;

void *justprint(void *data)
{
  int thread_id = *((int *)data);

  SSU_Sem_up(&par);
  for(int i=0; i < NUM_ITER; i++)
    {
      SSU_Sem_down(&sem);
      printf("This is thread %d\n", thread_id);
      SSU_Sem_up(&sem);
    }
  return 0;
}

int main(int argc, char *argv[])
{

  pthread_t mythreads[NUM_THREADS];
  int mythread_id[NUM_THREADS];

  SSU_Sem_init(&sem, 0);
  SSU_Sem_init(&par, 0);
  for(int i =0; i < NUM_THREADS; i++)
    {
      mythread_id[i] = i;
      pthread_create(&mythreads[i], NULL, justprint, (void *)&mythread_id[i]);
      SSU_Sem_down(&par);
    }
    SSU_Sem_up(&sem);
  for(int i =0; i < NUM_THREADS; i++)
    {
      pthread_join(mythreads[i], NULL);
    }
  
  return 0;
}
