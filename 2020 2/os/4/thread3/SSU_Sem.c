#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "SSU_Sem.h"

pthread_t getQueue_out(SSU_Sem *s){
	if(s->head->next != NULL){
		return s->head->next->thread;
	}
	return -1;
}

void Pop(SSU_Sem *s){
	struct waitNode * cu = s->head->next;
	if(cu != NULL){
		s->head->next = cu->next;
		free(cu);
	}
}

void Push(SSU_Sem *s){
	pthread_t self = pthread_self();
	struct waitNode * cur = s->head; 

	while(cur->next != NULL){
		cur = cur->next;
	}

	cur->next = (struct waitNode *)malloc(sizeof(struct waitNode));
	cur->next->thread = self;
	cur->next->next = NULL;
}

void SSU_Sem_init(SSU_Sem *s, int value) {
	s->count = value;
	s->head = (struct waitNode *)malloc(sizeof(struct waitNode));
	s->head->next = NULL;
	pthread_cond_init(&s->cond,NULL);
	pthread_mutex_init(&s->mutex,NULL);
}

void SSU_Sem_down(SSU_Sem *s) {
	pthread_t self = pthread_self();
	pthread_mutex_lock(&s->mutex);

	if(s->count <= 0 || getQueue_out(s) != -1){
		Push(s);
		while(1){
			pthread_cond_wait(&s->cond, &s->mutex);
			if(getQueue_out(s) ==  self){
				if(s->count > 0){
					Pop(s);
					break;
				}
			}
			else{
				pthread_cond_signal(&s->cond);
			}
		}
	}
	--s->count;
	pthread_mutex_unlock(&s->mutex);
}

void SSU_Sem_up(SSU_Sem *s) {
	pthread_mutex_lock(&s->mutex);
	pthread_cond_signal(&s->cond);
	++s->count;
	pthread_mutex_unlock(&s->mutex);
}
