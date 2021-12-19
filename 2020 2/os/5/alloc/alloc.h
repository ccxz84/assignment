#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define PAGESIZE 4096 //size of memory to allocate from OS
#define MINALLOC 8 //allocations will be 8 bytes or multiples of it

// function declarations
int init_alloc();
int cleanup();
char *alloc(int);
void dealloc(char *);

char * start;
char * astart;


struct aloc{
	int offset;
	int bytes;
	struct aloc * next;
};

struct aloc * head;
