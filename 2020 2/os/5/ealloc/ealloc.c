#include "ealloc.h"



void init_alloc(){

	if(head != NULL){
		cleanup();
	}
	if((head = mmap(0,PAGESIZE,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0))==(struct aloc *)-1){
		return;
	}
	if((head->addr = mmap(0,PAGESIZE,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0))==(caddr_t)-1){
		return;
	}
	if((free_head = mmap(0,PAGESIZE,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0))==(struct heap *)-1){
		return;
	}
	head->offset = 0;
	head->bytes = 0;
	head->maddr = (char *)head;
	head->next = NULL;
	free_head->heap = NULL;
	free_head->addr = (char *)free_head;
	free_head->next = NULL;
	num = 1;
	return;
}

struct heap * heapsearchNextFit(){
	struct heap * cur = free_head, * work;

	if(cur->next == NULL){
		work = (struct heap *)cur->addr + 1;
		work->addr = cur->addr;
		cur->next = work;
		return work;
	}

	if(cur->next != NULL && cur->next->next == NULL && ((char *)cur->next - cur->next->addr) >= sizeof(struct aloc)){
		work = (struct heap *)cur->next + 1;
		work->addr = cur->next->addr;
		cur->next->next = work;
		return work;
	}

	while(cur->next != NULL){
		if(cur->next->next == NULL){
			if(PAGESIZE - (((char *)cur->next - cur->next->addr) + sizeof(struct heap *)) < sizeof(struct heap)){
				if((work = mmap(0,PAGESIZE,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0))==(struct heap *)-1){
					return NULL;
				}
				work->addr = (char *)work;
				cur->next->next = work;
				//새로 페이지 할당
				return work;
			}
			else{
				work = (struct heap *)cur->next + 1;
				work->addr = cur->next->addr;
				cur->next->next = work;
				return work;
			}
		}
		else{

			if(((char *)cur->next - cur->next->addr) + (sizeof(struct heap) * 2) <= ((char *)cur->next->next-cur->next->next->addr)){
				
				work =(struct heap *)cur->next + 1;
				work->addr = cur->next->addr;
				cur->next->next = work;
				return work;
			}
		}
		cur = cur->next;
	}
	
	return NULL;
}

struct aloc * searchNextFit(){
	struct aloc * cur = head, * work;

	if(cur->next == NULL){
		work = (struct aloc *)cur->maddr + 1;
		work->maddr = cur->maddr;
		return work;
	}

	if(cur->next != NULL && cur->next->next == NULL && ((char *)cur->next - cur->next->maddr) >= sizeof(struct aloc)){
		work = (struct aloc *)cur->next + 1;
		work->maddr = cur->next->maddr;
		return work;
	}

	while(cur->next != NULL){
		if(cur->next->next == NULL){
			if(PAGESIZE - (((char *)cur->next - cur->next->maddr) + sizeof(struct aloc *)) < sizeof(struct aloc)){
				if((work = mmap(0,PAGESIZE,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0))==(struct aloc *)-1){
					return NULL;
				}
				work->maddr = (char *)work;
				//새로 페이지 할당
				return work;
			}
			else{
				work = (struct aloc *)cur->next + 1;
				work->maddr = cur->next->maddr;
				return work;
			}
		}
		else{

			if(((char *)cur->next - cur->next->maddr) + (sizeof(struct aloc) * 2) <= ((char *)cur->next->next-cur->next->next->maddr)){
				
				work =(struct aloc *)cur->next + 1;
				work->maddr = cur->next->maddr;
				return work;
			}
		}
		cur = cur->next;
	}
	
	return NULL;
}

char *alloc(int a){
	struct aloc * cur = head, *temp, *ttemp;
	int prevOffset = 0, nextOffset, offset = 0;
	struct heap * hcur = free_head;

	if(a <= 0){
		return NULL;
	}

	if(cur->next == NULL){
		cur->next = (struct aloc *)((char *)cur + sizeof(struct aloc));
		cur->next->addr = cur->addr;
		cur->next->offset = 0;
		cur->next->bytes = a;
		cur->next->maddr = cur->maddr;
		cur->next->next = NULL;
		return cur->next->addr + (cur->next->offset);
	}
	else{
		while(cur->next != NULL){
			prevOffset = cur->next->offset+cur->next->bytes;
			if(cur->next->next == NULL){
				if(PAGESIZE-prevOffset >= a){
					if((cur->next->next  = searchNextFit()) == NULL){ //관리를 위한 변수까지 할당
						//관리를 위한 페이지 새로 할당
						return NULL;
					}
					//관리를 위한 변수의 값 세팅 완료
					
					cur->next->next->next = NULL;
					cur->next->next->addr = cur->next->addr;
					cur->next->next->offset = cur->next->offset + cur->next->bytes;
					cur->next->next->bytes = a;

					

					return cur->next->next->addr + (cur->next->next->offset);
				}
				else{
					if((cur->next->next  = searchNextFit()) == NULL){ //관리를 위한 변수까지 할당
						//관리를 위한 페이지 새로 할당
						return NULL;
					}
					cur->next->next->next = NULL;

					if(hcur->next != NULL){
						cur->next->next->addr = hcur->next->heap;
						hcur->next = hcur->next->next;
					}
					else if(num >= 4 || (cur->next->next->addr = mmap(0,PAGESIZE,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0))==(char *)-1){
						return NULL;
					}

					cur->next->next->offset = 0;
					cur->next->next->bytes = a;
					num = num+1;
					//새로운 페이지 할당
					return cur->next->next->addr + (cur->next->next->offset);
				}
			}
			else{
				if(cur->next->addr == cur->next->next->addr){

					nextOffset = cur->next->next->offset;
					if(nextOffset - prevOffset >= a){

						temp = cur->next->next;
						if((cur->next->next = searchNextFit()) == NULL){
						//관리를 위한 페이지 새로 할당
							return NULL;
						}
						cur->next->next->next = temp;
						cur->next->next->addr = cur->next->addr;
						cur->next->next->offset = prevOffset;
						cur->next->next->bytes = a;
						return cur->next->next->addr + (cur->next->next->offset);
					//공간할당
					}
				}
				else{
					if(PAGESIZE-prevOffset >= a){
						temp = cur->next->next;
						if((cur->next->next  = searchNextFit(cur->next)) == NULL){ //관리를 위한 변수까지 할당
							//관리를 위한 페이지 새로 할당
							return NULL;
						}
						//관리를 위한 변수의 값 세팅 완료
						cur->next->next->next = temp;
						cur->next->next->addr = cur->next->addr;
						cur->next->next->offset = cur->next->offset + cur->next->bytes;
						cur->next->next->bytes = a;
						return cur->next->next->addr + (cur->next->next->offset);
					}
				}
			}
			cur = cur->next;
		}
	}



	
	return NULL;
}

void dealloc(char *ptr){
	struct aloc * cur = head, *temp;
	struct heap * hcur = free_head, *htemp;

	while(cur->next != NULL){
		if(ptr == cur->next->addr + cur->next->offset){
			if(((cur->next->next != NULL && cur->next->addr != cur->next->next->addr) || cur->next->next == NULL) && cur->next->addr != head->addr){
				htemp = heapsearchNextFit();
				htemp->heap = cur->next->addr;
			}
			temp = cur->next->next;
			cur->next = temp;
			break;
		}
		cur = cur->next;
	}

}

void cleanup(void){
	struct aloc * cur = head, *temp;
	struct heap * hcur = free_head;

	while(cur->next != NULL){
		if(cur->next->next == NULL){
			munmap(cur->next->addr, PAGESIZE);
			break;
		}

		temp = cur->next;

		if(temp->addr != temp->next->addr){
			munmap(temp->addr, PAGESIZE);
		}

		/*if(temp->maddr != temp->next->maddr){
			cur = cur->next->next;
			munmap(temp->maddr, PAGESIZE);
			continue;
		}*/

		cur = cur->next;		
	}
	while(hcur->next != NULL){
		munmap(hcur->next->heap, PAGESIZE);
		if(hcur->next->next == NULL){	
			munmap(hcur->next->addr, PAGESIZE);
			break;
		}
		if(hcur->next->addr != hcur->next->next->addr){
			munmap(hcur->next->addr,PAGESIZE);
		}
		hcur = hcur->next;
	}
}
