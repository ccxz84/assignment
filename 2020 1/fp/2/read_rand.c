#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define SUFFLE_NUM	10000

void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);

int main(int argc, char **argv)
{
    struct timeval startTime, endTime;
    
	int *read_order_list;
	int num_of_records;
    int fd;
    off_t fsize;

    if(argc < 2){
        fprintf(stderr, "Usage : %s file\n", argv[0]);
        exit(1);
    }
    
    if((fd = open(argv[1], O_RDONLY)) < 0){
        fprintf(stderr , "open error %s\n", argv[1]);
        exit(1);
    }
    fsize = lseek(fd, 0, SEEK_END);
    num_of_records = fsize / 100;
    read_order_list = (int *)malloc(sizeof(int) * num_of_records);
	GenRecordSequence(read_order_list, num_of_records);

    int i = 0;
    char tmp[100];
    gettimeofday(&startTime, NULL);
    for(;i<num_of_records;i++){
        lseek(fd, 100 * read_order_list[i], SEEK_SET);
        read(fd, tmp, 100);
    }
    gettimeofday(&endTime, NULL);
   	printf("#records: %ld timecost: %ld us\n",fsize/100,endTime.tv_usec - startTime.tv_usec); 


	return 0;
}

void GenRecordSequence(int *list, int n)
{
	int i, j, k;

	srand((unsigned int)time(0));

	for(i=0; i<n; i++)
	{
		list[i] = i;
	}
	
	for(i=0; i<SUFFLE_NUM; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}

	return;
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;

	return;
}
