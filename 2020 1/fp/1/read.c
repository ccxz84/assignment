#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char * argv[]){
	int offset;
	int read_bytes;
	int fd;	
	if(argc < 4){
		fprintf(stderr, "usage : %s file offest read_bytes\n", argv[0]);
		exit(1);
	}
	read_bytes = atoi(argv[3]);
	offset = atoi(argv[2]);
	char buf[read_bytes];
	memset(buf, 0, read_bytes);

	if((fd = open(argv[1], O_RDONLY)) < 0){
		fprintf(stderr, "open error %s file\n", argv[1]);
		exit(1);
	}
	if(lseek(fd, offset+1, SEEK_SET) < 0){
		fprintf(stderr, "lseek error\n");
		exit(1);
	}
	read(fd, buf, read_bytes);
	fprintf(stdout, "%s\n", buf);
	close(fd);
	exit(0);
}
