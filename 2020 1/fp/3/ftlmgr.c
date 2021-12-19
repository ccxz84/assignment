#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "flash.h"
// 필요한 경우 헤더파일을 추가한다

FILE *flashfp = NULL;	// fdevicedriver.c에서 사용
void flash_memory_emulator(char flashfile[], int block);
void flash_memory_write(char flashfile[], int ppn, char pagebuf[]);
void flash_memory_read(char flashfile[], int ppn, char sectorbuf[]);
void flash_memory_erase(char flashfile[], int pbn);
extern int dd_write(int ppn, char *pagebuf);
extern int dd_read(int ppn, char *pagebuf);
extern int dd_erase(int pbn);
//
// 이 함수는 FTL의 역할 중 일부분을 수행하는데 물리적인 저장장치 flash memory에 Flash device driver를 이용하여 데이터를
// 읽고 쓰거나 블록을 소거하는 일을 한다 (동영상 강의를 참조).
// flash memory에 데이터를 읽고 쓰거나 소거하기 위해서 fdevicedriver.c에서 제공하는 인터페이스를
// 호출하면 된다. 이때 해당되는 인터페이스를 호출할 때 연산의 단위를 정확히 사용해야 한다.
// 읽기와 쓰기는 페이지 단위이며 소거는 블록 단위이다.
// 
int main(int argc, char *argv[])
{	
	char sectorbuf[SECTOR_SIZE];
	char sparebuf[SPARE_SIZE];
	char pagebuf[PAGE_SIZE];
	char *blockbuf;
	int block;

	int i;
	int j;
	if(!strcmp(argv[1],"c")){
		block = atoi(argv[3]);
		flash_memory_emulator(argv[2],block);
	}
	else if(!strcmp(argv[1],"w")){
		j=0;
		memset((void *)pagebuf, (char)0xff, PAGE_SIZE);
		for(i = 0;i<strlen(argv[4]);i++){
			pagebuf[i] = argv[4][i];
		}
		for(i = SECTOR_SIZE;i<SECTOR_SIZE+strlen(argv[5]);i++){
			pagebuf[i] = argv[5][j];
			j++;
		}
		flash_memory_write(argv[2],atoi(argv[3]),pagebuf);
	}
	else if(!strcmp(argv[1],"r")){
		j=0;
		memset((void *)pagebuf, 0, PAGE_SIZE);
		flash_memory_read(argv[2],atoi(argv[3]),pagebuf);
		for(i=0;i<SECTOR_SIZE;i++){
			if(pagebuf[i] == (char)0xff){
				sectorbuf[i] = '\0';
				break;
			}
			sectorbuf[i] = pagebuf[i];
		}
		
		for(i=SECTOR_SIZE;i<PAGE_SIZE;i++){
			if(pagebuf[i] == (char)0xff){
				sparebuf[j] = '\0';
				break;
			}
			sparebuf[j] = pagebuf[i];
			j++;
		}
		if(strcmp(sectorbuf,"")&&strcmp(sparebuf,""))
			printf("%s %s\n",sectorbuf,sparebuf);
		
	}
	else if(!strcmp(argv[1], "e")){
		flash_memory_erase(argv[2], atoi(argv[3]));
	}
	// flash memory 파일 생성: 위에서 선언한 flashfp를 사용하여 flash 파일을 생성한다. 그 이유는 fdevicedriver.c에서 
	//                 flashfp 파일포인터를 extern으로 선언하여 사용하기 때문이다.
	// 페이지 쓰기: pagebuf의 섹터와 스페어에 각각 입력된 데이터를 정확히 저장하고 난 후 해당 인터페이스를 호출한다
	// 페이지 읽기: pagebuf를 인자로 사용하여 해당 인터페이스를 호출하여 페이지를 읽어 온 후 여기서 섹터 데이터와
	//                  스페어 데이터를 분리해 낸다
	// memset(), memcpy() 등의 함수를 이용하면 편리하다. 물론, 다른 방법으로 해결해도 무방하다.

	return 0;
}

void flash_memory_emulator(char flashfile[], int block){
	char blockbuf[BLOCK_SIZE];
	int ret;
	int i = 0;

	memset((void*)blockbuf, (char)0xff, BLOCK_SIZE);

	flashfp = fopen(flashfile,"w");

	for(i = 0;i < block; i++){
		fwrite((void *)blockbuf, BLOCK_SIZE, 1, flashfp);
	}
	fclose(flashfp);
}

void flash_memory_write(char flashfile[], int ppn, char pagebuf[]){
	flashfp = fopen(flashfile, "r+");
	dd_write(ppn,pagebuf);
	fclose(flashfp);
}

void flash_memory_read(char flashfile[], int ppn, char pagebuf[]){
	flashfp = fopen(flashfile, "r");
	dd_read(ppn,pagebuf);
	fclose(flashfp);
}

void flash_memory_erase(char flashfile[], int pbn){
	flashfp = fopen(flashfile, "r+");
	dd_erase(pbn);
	fclose(flashfp);
}