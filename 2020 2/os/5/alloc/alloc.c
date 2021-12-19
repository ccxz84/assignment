#include "alloc.h"

int init_alloc(){

	if(start != NULL || astart != NULL){
		cleanup();
	}

	if((start = mmap(0,PAGESIZE,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0))==(caddr_t)-1){
		return errno;
	}
	if((astart = mmap(0,PAGESIZE,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0))==(caddr_t)-1){
		return errno;
	}

	head = (struct aloc *)astart;
	head->offset = 0;
	head->bytes = 0;
	head->next = NULL;
	return 0;
}

int cleanup(){
	munmap(start,PAGESIZE);
	munmap(astart,PAGESIZE);
	return 0;
}

struct aloc * searchNextFit(){
	struct aloc * cur = head;
	struct aloc * ret;

	if(cur->next == NULL){
		ret = cur + 1;
		return ret;
	}

	if(cur->next != NULL && cur->next->next == NULL && (char *)cur->next - astart >= sizeof(struct aloc)){
		ret = cur->next + 1;
		return ret;
	}

	while(cur->next != NULL){
		if(cur->next->next == NULL){
			if(PAGESIZE - ((char *)cur->next - astart + sizeof(struct aloc)) < sizeof(struct aloc)){
				return NULL;
			}
			else{
				ret = cur->next + 1;
				return ret;
			}
		}
		else{
			if((char *)cur->next - astart + sizeof(struct aloc) + sizeof(struct aloc) <= (char *)cur->next->next - astart){
				ret = cur->next + 1;
				return ret;
			}
		}
		cur = cur->next;
	}
	
	return NULL;
}

char *alloc(int a){

	struct aloc * cur = head, *temp;
	int prevOffset = 0, nextOffset, offset = 0;

	if(start == NULL || astart == NULL){
		return NULL;
	}


	if(cur->next == NULL){
		cur->next = (struct aloc *)astart + 1;
		cur->next->offset = 0;
		cur->next->bytes = a;
		cur->next->next = NULL;
		return start+(cur->next->offset);
	}
	else{
		while(cur->next != NULL){
			prevOffset = cur->next->offset+cur->next->bytes;
			
			if(cur->next->next == NULL){
				if(PAGESIZE-prevOffset >= a){
					if((cur->next->next = searchNextFit()) == NULL){
						return NULL;
					}
					cur->next->next->next = NULL;
					cur->next->next->offset = cur->next->offset + cur->next->bytes;
					cur->next->next->bytes = a;
					return start + (cur->next->next->offset);
				}
			}
			else{
				nextOffset = cur->next->next->offset;
				if(nextOffset - prevOffset >= a){
					temp = cur->next->next;
					if((cur->next->next = searchNextFit()) == NULL){
						return NULL;
					}
					cur->next->next->next = temp;
					cur->next->next->offset = prevOffset;
					cur->next->next->bytes = a;
					return start + (cur->next->next->offset);
					//공간할당
				}
			}
			cur = cur->next;
		}
	}
	return NULL;
}

void dealloc(char * ptr){
	int offset = ptr-start;
	struct aloc * cur = head, *temp;

	while(cur->next != NULL){
		if(cur->next->offset == offset){
			temp = cur->next->next;
			cur->next = temp;
			break;
		}
		cur = cur->next;
	}
}

