#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUFFER_SIZE 100

int main(int argc, char *argv[]){
	int source;
	int target;
	char buf[BUFFER_SIZE];
	int length;

	if(argc < 3){
		fprintf(stderr, "usage : %s source target\n",argv[0]);
		exit(1);
	}

	if((source = open(argv[1], O_RDONLY)) < 0){
		fprintf(stderr, "Source File Open error\n");
		exit(1);
	}
	
	if((target = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644))<0){
		fprintf(stderr, "Target File Open error\n");
		exit(1);
	}
	while((length = read(source, buf, BUFFER_SIZE)) > 0){
		write(target, buf, length);
	}
	close(source);
	close(target);
	exit(0);
}
