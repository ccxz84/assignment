#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char * argv[]){ 
	int fd1;
	int fd2;
	off_t fsize;

	if(argc < 3){
		fprintf(stderr, "Usage : %s file1 file2\n", argv[0]);
		exit(1);
	}

	if((fd1 = open(argv[1], O_APPEND | O_WRONLY))< 0){
		fprintf(stderr, "open error %s\n", argv[1]);
		exit(1);
	}

	if((fd2 = open(argv[2], O_RDONLY))< 0){
		fprintf(stderr, "open error %s\n", argv[2]);
		exit(1);
	}
	if((fsize = lseek(fd2, 0, SEEK_END)) < 0){
		fprintf(stderr, "lseek error %s", argv[2]);
		exit(1);
	}
	char buf[fsize];
	memset(buf,0,fsize);
	lseek(fd2, 0, SEEK_SET);
	read(fd2, buf, fsize);
	write(fd1, buf,fsize);
	close(fd1);
	close(fd2);
	exit(0);
}
