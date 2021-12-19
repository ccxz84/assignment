#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "person.h"

#define record_unit (PAGE_SIZE / RECORD_SIZE)

int num,page_num;
Person * array;

void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, pagenum * PAGE_SIZE, SEEK_SET);
	fread(pagebuf,PAGE_SIZE,1,fp);	
}

void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, pagenum * PAGE_SIZE, SEEK_SET);
	fwrite(pagebuf,PAGE_SIZE,1,fp);
}

void _buildheap(char **heaparray,int idx){

	int i, cur;
	char temp[14];

	for(i=0;i<idx;i++){
		cur = i;
		while(atoll(heaparray[cur]) < atoll(heaparray[cur/2])){
			memset(temp, 0, 14);
			strcpy(temp, heaparray[cur]);
			strcpy(heaparray[cur],heaparray[cur/2]);
			strcpy(heaparray[cur/2],temp);
			cur = cur/2;
		}
	}
}

void unpack(const char *recordbuf, Person *p)
{
	char buf[RECORD_SIZE];
	char * ptr = buf;
	int i;

	memset(buf, 0, RECORD_SIZE);
	memcpy(buf, recordbuf,RECORD_SIZE);

	memset(p,0,sizeof(Person));
	ptr = strtok(ptr,"#");
	strcpy(p->sn,ptr);
	ptr = strtok(NULL,"#");
	strcpy(p->name,ptr);
	ptr = strtok(NULL,"#");
	strcpy(p->age,ptr);
	ptr = strtok(NULL,"#");
	strcpy(p->addr,ptr);
	ptr = strtok(NULL,"#");
	strcpy(p->phone,ptr);
	ptr = strtok(NULL,"#");
	strcpy(p->email,ptr);
}

void pack(char *recordbuf, const Person *p)
{
	memset(recordbuf,0xff, RECORD_SIZE);
	char * ptr = recordbuf;
	Person pbuf = *p;
	pbuf.sn[strlen(pbuf.sn)] = '#';
	pbuf.name[strlen(pbuf.name)] = '#';
	pbuf.age[strlen(pbuf.age)] = '#';
	pbuf.addr[strlen(pbuf.addr)] = '#';
	pbuf.phone[strlen(pbuf.phone)] = '#';
	pbuf.email[strlen(pbuf.email)] = '#';
	memcpy(ptr,pbuf.sn,strlen(pbuf.sn));
	ptr += strlen(pbuf.sn);
	memcpy(ptr,pbuf.name,strlen(pbuf.name));
	ptr += strlen(pbuf.name);
	memcpy(ptr,pbuf.age,strlen(pbuf.age));
	ptr += strlen(pbuf.age);
	memcpy(ptr,pbuf.addr,strlen(pbuf.addr));
	ptr += strlen(pbuf.addr);
	memcpy(ptr,pbuf.phone,strlen(pbuf.phone));
	ptr += strlen(pbuf.phone);
	memcpy(ptr,pbuf.email,strlen(pbuf.email));
	ptr += strlen(pbuf.email);
	
}

void insert(FILE *fp, const Person *p)
{
	char header[PAGE_SIZE] , recordbuf[RECORD_SIZE], pagebuf[PAGE_SIZE];
	char * ptr = header, *tmp;
	off_t size;
	int pagenum,recordnum, del_pagenum, del_recordnum, remain, pagetmp,x;
	
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	pack(recordbuf,p);
	memset(header,(char)0xff, PAGE_SIZE);
	if(size < 256){
		pagenum = 1;
		recordnum = 0;
		del_pagenum = -1;
		del_recordnum = -1;
		memcpy(ptr,(char*)&(pagenum), sizeof(pagenum));
		ptr += sizeof(pagenum);
		memcpy(ptr,(char*)&(recordnum), sizeof(recordnum));
		ptr += sizeof(recordnum);
		memcpy(ptr,(char*)&(del_pagenum), sizeof(del_pagenum));
		ptr += sizeof(del_pagenum);
		memcpy(ptr,(char*)&(del_recordnum), sizeof(del_recordnum));
		writePage(fp,header,0);
	}
	ptr = header;
	readPage(fp,header,0);
	memcpy(&pagenum, ptr, sizeof(pagenum));
	ptr += sizeof(pagenum);
	memcpy(&recordnum,ptr,sizeof(recordnum));
	ptr += sizeof(recordnum);
	memcpy(&del_pagenum,ptr,sizeof(del_pagenum));
	ptr += sizeof(del_pagenum);
	memcpy(&del_recordnum,ptr,sizeof(del_recordnum));

	ptr = pagebuf;
	if(del_pagenum == -1 && del_recordnum == -1){
		if((remain = recordnum % record_unit) > 0){
			readPage(fp,pagebuf,pagenum-1);
			ptr += remain * RECORD_SIZE;
			memcpy(	ptr, recordbuf,RECORD_SIZE);
			writePage(fp,pagebuf,pagenum-1);
			++recordnum;
			ptr = header;
			ptr += sizeof(pagenum);
			memcpy(ptr, (char *)&(recordnum),sizeof(recordnum));
			writePage(fp, header,0);
		}
		else{
			++pagenum;
			++recordnum;
			memset(pagebuf, (char)0xff,PAGE_SIZE);
			memcpy(ptr, recordbuf,RECORD_SIZE);
			writePage(fp,pagebuf,pagenum-1);
			ptr = header;
			memcpy(ptr, (char *)&(pagenum),sizeof(pagenum));
			ptr += sizeof(pagenum);
			memcpy(ptr, (char *)&(recordnum),sizeof(recordnum));
			writePage(fp, header, 0);	
		}
	}
	else{
		readPage(fp,pagebuf,del_pagenum);
		ptr += RECORD_SIZE * del_recordnum;
		tmp = ptr;
		pagetmp = del_pagenum;
		if(*ptr != '*')
			printf("error?\n");
		++tmp;
		memcpy(&del_pagenum, tmp, sizeof(del_pagenum));
		tmp += sizeof(del_pagenum);
		memcpy(&del_recordnum,tmp,sizeof(del_recordnum));
		memcpy(ptr, recordbuf, RECORD_SIZE);
		writePage(fp,pagebuf,pagetmp);
		ptr = header;
		ptr += (sizeof(pagenum) + sizeof(recordnum));
		memcpy(ptr, (char *)&(del_pagenum),sizeof(del_pagenum));
		ptr += sizeof(del_pagenum);
		memcpy(ptr, (char *)&(del_recordnum),sizeof(del_recordnum));
		writePage(fp, header, 0);
	}
}

void buildHeap(FILE *inputfp, char **heaparray)
{
	char header[PAGE_SIZE],pagebuf[PAGE_SIZE],recordbuf[RECORD_SIZE];
	int h_pagenum, h_recordnum, pagenum,recordnum, idx = 0, i =0;
	char *ptr;
	Person tmp;

	ptr = header;
	readPage(inputfp,header,0);
	memcpy(&h_pagenum, ptr, sizeof(h_pagenum));
	ptr += sizeof(h_pagenum);
	memcpy(&h_recordnum,ptr,sizeof(h_recordnum));

	for(pagenum = 1;pagenum < h_pagenum;pagenum++){
		readPage(inputfp,pagebuf,pagenum);
		ptr = pagebuf;

		i = 0;
		for(recordnum = 0;recordnum< record_unit && h_recordnum > (((pagenum-1) *record_unit) + recordnum) ;recordnum++){
			i++;
			ptr += recordnum * RECORD_SIZE;
			strncpy(recordbuf,ptr,RECORD_SIZE);
			unpack(recordbuf, &tmp);
			array[((pagenum-1) *record_unit) + recordnum] = tmp;
			strcpy(heaparray[idx],tmp.sn);
			idx++;
			_buildheap(heaparray,idx);
			
		}
	}
}

void makeSortedFile(FILE *outputfp, char **heaparray)
{
	int i,j, idx = num-1, pagenum, recordnum, del_pagenum, del_recordnum;
	char temp[14],recordbuf[RECORD_SIZE];

	for(i=0;i<num;i++){
		strcpy(temp, heaparray[0]);
		for(j = 0;j<num;j++){
			
			if(!strcmp(heaparray[0],array[j].sn))
			{
				insert(outputfp,&array[j]);
			}
		}



		strcpy(heaparray[0], heaparray[idx]);
		strcpy(heaparray[idx],"");
		
		_buildheap(heaparray,idx);
		idx--;
	}
}

int main(int argc, char *argv[])
{
	FILE *inputfp;
	FILE *outputfp;
	char ** heap, *ptr,header[PAGE_SIZE];
	int h_pagenum, h_recordnum,i;

	inputfp = fopen(argv[2],"r");
	outputfp = fopen(argv[3], "w+");

	ptr = header;
	readPage(inputfp,header,0);
	memcpy(&h_pagenum, ptr, sizeof(h_pagenum));
	ptr += sizeof(h_pagenum);
	memcpy(&h_recordnum,ptr,sizeof(h_recordnum));

	page_num = h_pagenum;

	heap = (char **)malloc(sizeof(char *)*h_recordnum);
	array = malloc(sizeof(Person) * h_recordnum);

	memset(array, 0, sizeof(Person) * h_recordnum);
	num = h_recordnum;

	for(i = 0;i<h_recordnum;i++){
		heap[i] = malloc(14);
	}

	buildHeap(inputfp, heap);
	makeSortedFile(outputfp,heap);

	//for(i = 0; i<num;i++)
		//printf("%s\n",array[i].sn);

	return 1;
}