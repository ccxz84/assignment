#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char *argv[]){
	int fd;
	int offset;
	off_t fsize;
	int buf_size;
	if(argc < 4){
		fprintf(stderr, "Usage : %s file offset data\n", argv[0]);
		exit(1);
	}

	offset = atoi(argv[2]);

	if((fd = open(argv[1], O_RDWR)) < 0){
		fprintf(stderr, "open error %s\n", argv[1]);
		exit(1);
	}

	if((fsize = lseek(fd, 0, SEEK_END)) < 0){
		fprintf(stderr, "lseek error\n");
		exit(1);
	}
	offset = offset < fsize ? offset : fsize;
	lseek(fd, offset, SEEK_SET);

	buf_size = offset < fsize ? fsize - offset : 0;

	char buf[buf_size];
	memset(buf, 0, buf_size);

	read(fd, buf, buf_size);
	write(fd, argv[3], strlen(argv[3]));
	write(fd, buf, buf_size);
	close(fd);
	exit(0);
}
