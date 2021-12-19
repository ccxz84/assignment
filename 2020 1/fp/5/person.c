#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "person.h"

#define record_unit (PAGE_SIZE / RECORD_SIZE)

//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉 페이지 단위로 읽거나 써야 합니다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, pagenum * PAGE_SIZE, SEEK_SET);
	fread(pagebuf,PAGE_SIZE,1,fp);	
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, pagenum * PAGE_SIZE, SEEK_SET);
	fwrite(pagebuf,PAGE_SIZE,1,fp);
	
}

//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 그런 후 이 레코드를 저장할 페이지를 readPage()를 통해 프로그램 상에
// 읽어 온 후 pagebuf에 recordbuf에 저장되어 있는 레코드를 저장한다. 그 다음 writePage() 호출하여 pagebuf를 해당 페이지 번호에
// 저장한다. pack() 함수에서 readPage()와 writePage()를 호출하는 것이 아니라 pack()을 호출하는 측에서 pack() 함수 호출 후
// readPage()와 writePage()를 차례로 호출하여 레코드 쓰기를 완성한다는 의미이다.
// 
void pack(char *recordbuf, const Person *p)
{
	char * ptr = recordbuf;
	Person pbuf = *p;
	pbuf.sn[strlen(pbuf.sn)] = '#';
	pbuf.name[strlen(pbuf.name)] = '#';
	pbuf.age[strlen(pbuf.age)] = '#';
	pbuf.addr[strlen(pbuf.addr)] = '#';
	pbuf.phone[strlen(pbuf.phone)] = '#';
	pbuf.email[strlen(pbuf.email)] = '#';
	memcpy(ptr,pbuf.sn,sizeof(pbuf.sn));
	ptr += sizeof(pbuf.sn);
	memcpy(ptr,pbuf.name,sizeof(pbuf.name));
	ptr += sizeof(pbuf.name);
	memcpy(ptr,pbuf.age,sizeof(pbuf.age));
	ptr += sizeof(pbuf.age);
	memcpy(ptr,pbuf.addr,sizeof(pbuf.addr));
	ptr += sizeof(pbuf.addr);
	memcpy(ptr,pbuf.phone,sizeof(pbuf.phone));
	ptr += sizeof(pbuf.phone);
	memcpy(ptr,pbuf.email,sizeof(pbuf.email));
	ptr += sizeof(pbuf.email);
}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다. 이 함수가 언제 호출되는지는
// 위에서 설명한 pack()의 시나리오를 참조하면 된다.
//
void unpack(const char *recordbuf, Person *p)
{
	char buf[RECORD_SIZE];
	char * ptr = buf;
	strcpy(buf,recordbuf);
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

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값을 구조체에 저장한 후 아래의 insert() 함수를 호출한다.
//
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

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *sn)
{
	char header[PAGE_SIZE],pagebuf[PAGE_SIZE],recordbuf[RECORD_SIZE], data_sn[14];
	int h_pagenum, h_recordnum, del_recordnum, del_pagenum, pagenum,recordnum, flag = 0;
	char *ptr;

	ptr = header;
	readPage(fp,header,0);
	memcpy(&h_pagenum, ptr, sizeof(h_pagenum));
	ptr += sizeof(h_pagenum);
	memcpy(&h_recordnum,ptr,sizeof(h_recordnum));
	ptr += sizeof(h_recordnum);
	memcpy(&del_pagenum,ptr,sizeof(del_pagenum));
	ptr += sizeof(del_pagenum);
	memcpy(&del_recordnum,ptr,sizeof(del_recordnum));

	for(pagenum = 1;pagenum < h_pagenum;pagenum++){
		if(flag == 1){
			break;
		}
		readPage(fp,pagebuf,pagenum);
		ptr = pagebuf;

		for(recordnum = 0;recordnum< record_unit;recordnum++){
			ptr += recordnum * RECORD_SIZE;
			memcpy(data_sn,ptr,sizeof(data_sn));
			data_sn[strlen(data_sn)-1] = '\0';
			if(!strcmp(data_sn,sn)){
				flag = 1;
				--pagenum;
				break;
			}
		}
	}
	if(pagenum >= h_pagenum){
		printf("record not fount\n");
		return;
	}
	memcpy(ptr,"*",1);
	++ptr;
	memcpy(ptr,(char *)&(del_pagenum),sizeof(del_pagenum));
	ptr += sizeof(del_pagenum);
	memcpy(ptr,(char *)&(del_recordnum),sizeof(del_recordnum));
	writePage(fp,pagebuf,pagenum);
	del_recordnum = recordnum;
	del_pagenum = pagenum;
	ptr = header;
	ptr += (sizeof(h_pagenum)+sizeof(h_recordnum));
	memcpy(ptr,(char *)&(del_pagenum),sizeof(del_pagenum));
	ptr += sizeof(del_pagenum);
	memcpy(ptr,(char *)&(del_recordnum),sizeof(del_recordnum));
	writePage(fp,header,0);
}

int main(int argc, char *argv[])
{
	FILE *fp;  // 레코드 파일의 파일 포인터
	Person p;
	char recordbuf[RECORD_SIZE];

	if((fp = fopen(argv[2],"r+")) == NULL){
		fp = fopen(argv[2],"w+");
		fclose(fp);
		fp = fopen(argv[2],"r+");
	}

	if(!strcmp(argv[1],"i")){
		memset(p.sn,0,sizeof(p.sn));
		memset(p.name,0,sizeof(p.name));
		memset(p.age,0,sizeof(p.age));
		memset(p.addr,0,sizeof(p.addr));
		memset(p.phone,0,sizeof(p.phone));
		memset(p.email,0,sizeof(p.email));
		strcpy(p.sn,argv[3]);
		strcpy(p.name,argv[4]);
		strcpy(p.age,argv[5]);
		strcpy(p.addr,argv[6]);
		strcpy(p.phone,argv[7]);
		strcpy(p.email,argv[8]);
		insert(fp,&p);
	}
	else if(!strcmp(argv[1],"d")){
		delete(fp,argv[3]);
	}
	else{
		printf("unknown command\n");
	}
	fclose(fp);

	
	return 1;
}