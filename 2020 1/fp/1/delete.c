#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
	int fd;
	int offset;
	int bytes;
	off_t fsize;

	if(argc < 4){
		fprintf(stderr, "Usage : %s file offset bytes", argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_RDONLY)) < 0){
		fprintf(stderr, "open error %s\n", argv[1]);
		exit(1);
	}

	offset = atoi(argv[2]);
	bytes = atoi(argv[3]);

	if((fsize = lseek(fd, 0, SEEK_END)) < 0){
		fprintf(stderr, "lseek error\n");
		exit(1);
	}
	offset = offset < fsize ? offset+1 : fsize;
	char pre_data[offset];
	int remove_offset = (offset + bytes+1) < fsize ? fsize - (offset + bytes+1) : 0;
	memset(pre_data, 0 , offset);
	char post_data[remove_offset];
	memset(post_data, 0 , remove_offset);
	lseek(fd, 0, SEEK_SET);
	read(fd, pre_data, offset);
	lseek(fd,offset+ bytes, SEEK_SET);
	read(fd, post_data, remove_offset);

	close(fd);

	if((fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0){
		fprintf(stderr, "creat error %s\n", argv[1]);
		exit(1);
	}

	write(fd, pre_data, offset);
	write(fd, post_data, remove_offset);

	close(fd);

	exit(0);
}
	

