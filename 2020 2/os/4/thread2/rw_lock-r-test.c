#include "rw_lock.h"

void init_rwlock(struct rw_lock * rw)
{
  //	Write the code for initializing your read-write lock.
	pthread_cond_init(&rw->rcond,NULL);
	pthread_cond_init(&rw->wcond,NULL);
	pthread_mutex_init(&rw->mutex,NULL);
	rw->lock = UNLOCK;
	rw->head = (struct node *)malloc(sizeof(struct node));
	rw->head->next = NULL;
	rw->locknum = 0;
	rw->writeblock = 0;
}

void r_lock(struct rw_lock * rw)
{
	int i;
	pthread_t self = pthread_self();
	struct node * current = rw->head;
  //	Write the code for aquiring read-write lock by the reader.
	pthread_mutex_lock(&rw->mutex);

	while(rw->lock == WRITELOCK){
		pthread_cond_wait(&rw->rcond, &rw->mutex);
	}

	while(current->next != NULL){
		current = current->next;
	}

	rw->lock = READLOCK;
	current->next = (struct node *)malloc(sizeof(struct node));
	current->next->thread = self;
	++rw->locknum;
	pthread_mutex_unlock(&rw->mutex);

}

void r_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
	int i;
	pthread_t self = pthread_self();
	struct node * current = rw->head;
	struct node * next = NULL;

	pthread_mutex_lock(&rw->mutex);
	while(current->next != NULL){
		if(current->next->thread == self){
			break;
		}
		current = current->next;
	}

	if(current->next != NULL){
		next = current->next->next;
		free(current->next);
		current->next = next;
		--rw->locknum;
		if(rw->locknum <= 0){
			rw->lock = UNLOCK;
			if(rw->writeblock > 0){
				pthread_cond_signal(&rw->wcond);
			}
			else{
				pthread_cond_signal(&rw->rcond);
			}
		}
		pthread_cond_signal(&rw->rcond);
	}

	pthread_mutex_unlock(&rw->mutex);

}

void w_lock(struct rw_lock * rw)
{
	int i, flag = 0;
	pthread_t self = pthread_self();
  //	Write the code for aquiring read-write lock by the writer.
	pthread_mutex_lock(&rw->mutex);
	if(rw->lock == READLOCK || rw->locknum > 0){
		flag = 1;
		++rw->writeblock;
	}
	while(rw->lock == READLOCK || rw->locknum > 0){
		pthread_cond_wait(&rw->wcond, &rw->mutex);
	}

	if(!(rw->lock == READLOCK || rw->locknum > 0) && flag == 1){
		--rw->writeblock;
	}

	rw->lock = WRITELOCK;
	rw->locknum = 1;
	rw->head->next = (struct node *)malloc(sizeof(struct node));
	rw->head->next->thread = self;
	rw->head->next->next = NULL;
	pthread_mutex_unlock(&rw->mutex);
}

void w_unlock(struct rw_lock * rw)
{
	int i;
	pthread_t self = pthread_self();
  //	Write the code for releasing read-write lock by the writer.
	pthread_mutex_lock(&rw->mutex);

	if(rw->head->next->thread == self){
		rw->lock = UNLOCK;
		rw->locknum = 0;
		free(rw->head->next);
		rw->head->next = NULL;
		if(rw->writeblock > 0){
			pthread_cond_signal(&rw->wcond);
		}
		else{
			pthread_cond_signal(&rw->rcond);
		}
	}
	pthread_mutex_unlock(&rw->mutex);
}

