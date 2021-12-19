#include <pthread.h>

struct waitNode{
	pthread_t thread;
	struct waitNode * next;
};


typedef struct SSU_Sem {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int count;
	struct waitNode * head;
} SSU_Sem;


void SSU_Sem_init(SSU_Sem *, int);
void SSU_Sem_up(SSU_Sem *);
void SSU_Sem_down(SSU_Sem *);
