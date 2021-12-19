// 주의사항
// 1. sectormap.h에 정의되어 있는 상수 변수를 우선적으로 사용해야 함
// 2. sectormap.h에 정의되어 있지 않을 경우 본인이 이 파일에서 만들어서 사용하면 됨
// 3. 필요한 data structure가 필요하면 이 파일에서 정의해서 쓰기 바람(sectormap.h에 추가하면 안됨)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "sectormap.h"

extern int dd_write(int ppn, char *pagebuf);
extern int dd_read(int ppn, char *pagebuf);
extern int dd_erase(int pbn);

int get_queue();
int inset_queue(int data);

// 필요한 경우 헤더 파일을 추가하시오.

//
// flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
// 초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
// file system에 의해 반드시 먼저 호출이 되어야 한다.
//

struct ppn_list{
	int ppn;
	struct ppn_list *next;
};

struct Queue{
	int queue[DATABLKS_PER_DEVICE*PAGES_PER_BLOCK];
	int front;
	int end;
	int is_empty;
};

int free_block;
struct ppn_list *head;
struct ppn_list *tail;
struct Queue que;
int * adress_mapping_table;

void ftl_open()
{
	int i = 0;
	//
	// address mapping table 초기화
	// free block's pbn 초기화
    	// address mapping table에서 lbn 수는 DATABLKS_PER_DEVICE 동일
	adress_mapping_table = (int *)malloc(sizeof(int)*DATABLKS_PER_DEVICE*PAGES_PER_BLOCK);
	memset(adress_mapping_table,-1,sizeof(int)*DATABLKS_PER_DEVICE*PAGES_PER_BLOCK);
	free_block = BLOCKS_PER_DEVICE-1;
	head = (struct ppn_list *)malloc(sizeof(struct ppn_list));
	tail = head;

	que.front = 0;
	que.end = (DATABLKS_PER_DEVICE*PAGES_PER_BLOCK) -1;
	que.is_empty = 1;
	for(i = 0;i<DATABLKS_PER_DEVICE*PAGES_PER_BLOCK;i++){
		que.queue[DATABLKS_PER_DEVICE*PAGES_PER_BLOCK - 1 - i] = i;
	}
	return;
}

//
// 이 함수를 호출하기 전에 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 한다.
// 즉, 이 함수에서 메모리를 할당받으면 안된다.
//
void ftl_read(int lsn, char *sectorbuf)
{
	int ppn = adress_mapping_table[lsn];
	char pagebuf[PAGE_SIZE];
	if(ppn == -1){
		fprintf(stderr,"empty page\n");
		return;
	}
	dd_read(ppn,pagebuf);
	memcpy(sectorbuf,pagebuf,SECTOR_SIZE);
	return;
}


void ftl_write(int lsn, char *sectorbuf)
{
	char pagebuf[PAGE_SIZE];
	int ppn = adress_mapping_table[lsn];
	int i = 0;
	struct ppn_list * node;
	char buf[SPARE_SIZE];
	char * csparebuf = buf;
	SpareData sparebuf;

	if(ppn != -1){
		tail->next = (struct ppn_list *)malloc(sizeof(struct ppn_list));
		tail = tail->next;
		tail->ppn = ppn;
		tail->next = NULL;
	}

	if(que.is_empty){
		ppn = get_queue();
		adress_mapping_table[lsn] = ppn;
	}
	else{
		if(tail == head){
			printf("capacity error\n");
			return;
		}
		int pbn = head->next->ppn/PAGES_PER_BLOCK;
		int i,j;
		struct ppn_list * free_node;
		
		for(i = 0;i<PAGES_PER_BLOCK;i++){
			if(head->next->ppn != i + PAGES_PER_BLOCK*pbn){
				dd_read(i+ PAGES_PER_BLOCK*pbn,pagebuf);
				dd_write(free_block*PAGES_PER_BLOCK+i,pagebuf);
				for(j=0;j<DATABLKS_PER_DEVICE*PAGES_PER_BLOCK;j++){
					if(adress_mapping_table[j] == i + PAGES_PER_BLOCK*pbn){
						adress_mapping_table[j] = free_block*PAGES_PER_BLOCK + i;
						break;
					}
				}
			}
		}
		dd_erase(pbn);
		adress_mapping_table[lsn] = ((head->next->ppn)%PAGES_PER_BLOCK)+free_block*PAGES_PER_BLOCK;
		ppn = ((head->next->ppn)%PAGES_PER_BLOCK)+free_block*PAGES_PER_BLOCK;
		free_node = head;
		head = head->next;
		free(free_node);
		free_block = pbn;
	}

	sparebuf.lpn = lsn;
	sparebuf.is_invalid = 1;
	memset(sparebuf.dummy,(char)0xff,(SPARE_SIZE - 8));

	memset(csparebuf,0,SPARE_SIZE);
	memcpy(csparebuf,(char*)&(sparebuf.lpn),sizeof(sparebuf.lpn));
	csparebuf += sizeof(sparebuf.lpn);
	memcpy(csparebuf,(char*)&(sparebuf.is_invalid),sizeof(sparebuf.is_invalid));
	csparebuf += sizeof(sparebuf.is_invalid);
	memcpy(csparebuf,sparebuf.dummy,(SPARE_SIZE - 8));

	memcpy(pagebuf,sectorbuf,SECTOR_SIZE);
	memcpy(pagebuf+SECTOR_SIZE,csparebuf,SPARE_SIZE);

	dd_write(ppn,pagebuf);

	return;
}

void ftl_print()
{
	int i = 0;
	printf("lpn	  ppn\n");
	for(;i<DATABLKS_PER_DEVICE*PAGES_PER_BLOCK;i++){
		printf("%d 	%d\n",i,adress_mapping_table[i]);
	}
	printf("free block's pbn = %d\n",free_block);
	return;
}

int get_queue(){
	int ret = que.queue[que.front];
	que.front++;
	que.front = que.front% (DATABLKS_PER_DEVICE*PAGES_PER_BLOCK);
	if((que.front == (que.end + 1)) || que.front == ((que.end+ 1) % (DATABLKS_PER_DEVICE*PAGES_PER_BLOCK))){
		que.is_empty = 0;
	}
	return ret;
}

int inset_queue(int data){
	que.end++;
	que.end = que.end % (DATABLKS_PER_DEVICE*PAGES_PER_BLOCK);
	que.queue[que.end] = data;
	que.is_empty = 0;
}