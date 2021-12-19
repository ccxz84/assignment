#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int item_to_produce, curr_buf_size;
int total_items, max_buf_size, num_workers, num_masters;

int *buffer;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int waitc, runThreadnum;

void print_produced(int num, int master) {

  printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) {

  printf("Consumed %d by worker %d\n", num, worker);
  
}

void *run_worker_thread(void *data){
	int i;
	int thread_id = *((int *)data);
	++waitc;
	++runThreadnum;

	if(waitc >= num_masters + num_workers){
		pthread_mutex_lock(&mutex);
  		pthread_cond_signal(&cond);
    	pthread_mutex_unlock(&mutex);
	}

	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&cond, &mutex);
	while(1){
		for(i = 0; i < max_buf_size; i++){
			if(buffer[i] != -1){
				break;
			}
		}

		if(i >= max_buf_size){
			break;
		}
		else{
			print_consumed(buffer[i], thread_id);
			buffer[i] = -1;
		}
		pthread_cond_signal(&cond);
		if(runThreadnum > 1)
			pthread_cond_wait(&cond, &mutex);
	}
	pthread_cond_signal(&cond);
	--runThreadnum;
	pthread_mutex_unlock(&mutex);
	return 0;
}

//produce items and place in buffer
//modify code below to synchronize correctly
void *generate_requests_loop(void *data)
{
	int i;
	int thread_id = *((int *)data);
	++waitc;
	++runThreadnum;


	if(waitc >= num_masters + num_workers){
		pthread_mutex_lock(&mutex);
  		pthread_cond_signal(&cond);
    	pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&cond, &mutex);
	while(1)
    {		
    	if(item_to_produce >= total_items) {
			break;
		}
		for(curr_buf_size = 0; curr_buf_size < max_buf_size; curr_buf_size++){
			if(buffer[curr_buf_size] == -1)
				break;
		}

		if(!(curr_buf_size >= max_buf_size)){
        	buffer[curr_buf_size] = item_to_produce;
			print_produced(item_to_produce, thread_id);
			item_to_produce++;
		}
		
		pthread_cond_signal(&cond);
		if(runThreadnum > 1)
			pthread_cond_wait(&cond, &mutex);
	}
	pthread_cond_signal(&cond);
	--runThreadnum;
	pthread_mutex_unlock(&mutex);
	return 0;
}

//write function to be run by worker threads
//ensure that the workers call the function print_consumed when they consume an item

int main(int argc, char *argv[])
{
	int *master_thread_id;
	pthread_t *master_thread;
	int *worker_thread_id;
 	pthread_t *worker_thread;
	item_to_produce = 0;
	curr_buf_size = 0;
	waitc = 0;
	runThreadnum = 0;
  
	int i;
  
	if (argc < 5) {
		printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
		exit(1);
	}
	else {
		num_masters = atoi(argv[4]);
		num_workers = atoi(argv[3]);
		total_items = atoi(argv[1]);
		max_buf_size = atoi(argv[2]);
	}
    

	buffer = (int *)malloc (sizeof(int) * max_buf_size);

	memset(buffer,-1,sizeof(int) * max_buf_size);

	//create master producer threads
	master_thread_id = (int *)malloc(sizeof(int) * num_masters);
	master_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_masters);
	for (i = 0; i < num_masters; i++)
		master_thread_id[i] = i;

	for (i = 0; i < num_masters; i++)
		pthread_create(&master_thread[i], NULL, generate_requests_loop, (void *)&master_thread_id[i]);
  
	//create worker consumer threads
	worker_thread_id = (int *)malloc(sizeof(int) * num_workers);
   	worker_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_workers);
   	for (i = 0; i < num_workers; i++)
		worker_thread_id[i] = i;

	for (i = 0; i < num_workers; i++)
		pthread_create(&worker_thread[i], NULL, run_worker_thread, (void *)&worker_thread_id[i]);

	//wait for all threads to complete
	for (i = 0; i < num_masters; i++)
    {
		pthread_join(master_thread[i], NULL);
		printf("master %d joined\n", i);
    }

    for (i = 0; i < num_workers; i++)
    {
		pthread_join(worker_thread[i], NULL);
		printf("worker %d joined\n", i);
    }
  
	/*----Deallocating Buffers---------------------*/
	free(buffer);
	free(master_thread_id);
	free(master_thread);
  
	return 0;
}
