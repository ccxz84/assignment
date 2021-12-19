#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define READLOCK 1
#define WRITELOCK 2
#define UNLOCK 0
#define ACCESSLOCKNUM 3

struct rw_lock
{
	pthread_mutex_t mutex;
	pthread_cond_t rcond;
	pthread_cond_t wcond;
	int lock;
	struct node * head;
	int locknum;
	int writeblock;
};

struct node {
	pthread_t thread;
	struct node * next;
};

void init_rwlock(struct rw_lock * rw);
void r_lock(struct rw_lock * rw);
void r_unlock(struct rw_lock * rw);
void w_lock(struct rw_lock * rw);
void w_unlock(struct rw_lock * rw);
long *max_element(long* start, long* end);
long *min_element(long* start, long* end);
