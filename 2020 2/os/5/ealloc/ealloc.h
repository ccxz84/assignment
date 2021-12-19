#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

//granularity of memory to mmap from OS
#define PAGESIZE 4096 

//minimum allocation size
#define MINALLOC 256

// function declarations to support
void init_alloc(void);
char *alloc(int);
void dealloc(char *);
void cleanup(void);

int num;

struct aloc{
	char * addr;
	int offset;
	int bytes;
	char * maddr;
	struct aloc * next;
};

struct heap{
	char * heap;
	char * addr;
	struct heap * next;
};

struct heap * free_head;
struct aloc * head;