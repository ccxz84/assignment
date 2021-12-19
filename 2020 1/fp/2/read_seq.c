#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    struct timeval startTime, endTime;
    off_t fsize;
    
    int fd; 
    if(argc < 2){
        fprintf(stderr, "Usage : %s file\n", argv[0]);
        exit(1);
    }
    if((fd = open(argv[1], O_RDONLY)) < 0){
        fprintf(stderr, "open error %s\n", argv[1]);
        exit(1);
    }
    fsize = lseek(fd, 0, SEEK_END);

    int i = 0;
    char tmp[100];
    gettimeofday(&startTime, NULL);
    for(;i<fsize/100;i++){
      lseek(fd, 100 * i, SEEK_SET);
      read(fd,tmp,100);
    }

    gettimeofday(&endTime,NULL);
    printf("#records: %ld timecost: %ld us\n",fsize/100,endTime.tv_usec - startTime.tv_usec);
    exit(1);

	return 0;
}
