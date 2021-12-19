#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include "ssu_mntr.h"

int tree_command(char (*command_token)[BUFLEN]){
	char * path;

	printf("%s",DIRECTORY); //디렉터리의 파일명을 먼저 출력
	print_tree(DIRECTORY,direc_filter,0,strlen(DIRECTORY),0); //트리구조 출력
	printf("\n");
}

int print_tree(char * path,int (*filter)(const struct dirent *), int phase,int blank, int flag){
	char cwd[FILELEN];
	struct dirent ** namelist, *dirp;
	int i,j,count,dir=0;
	struct stat statbuf;
	char buf[BUFLEN];
	char * _realpath;
	char cPath[FILELEN];
	int compare_flag;

	strcpy(cPath,path);//인자의 경로를 cPath에 복사

	_realpath = realpath(path,NULL); //path를 절대경로로 변경

	getcwd(cwd,FILELEN); //현재 작업디렉터리 백업
	chdir(_realpath); //작업디렉터리 변경
	count = scandir(_realpath,&namelist,filter,alphasort); //경로에 대해 scandir 수행

	if(count == 0){//파일이 없으면 함수 종료
		printf("\n");
		chdir(cwd);
		return 1;
	}
	phase++; //함수의 실행횟수를 증가시킴

	for(i=0;i<count;i++){ //파일에 대해 반복수행
		dirp = namelist[i];
		compare_flag = 1; //비교 플래그를 1로 설정
		if(lstat(dirp->d_name,&statbuf)){ //파일의 stat 구조체를 가져옴
			fprintf(stderr,"stat error\n");
			break;
		}

		if(i != 0){ //첫번째 반복이면
			strcpy(buf,""); //buf에 공백을 넣고
			for(j= 0;phase>j;j++,compare_flag *= 2){//phase 수 만큼 반복을 수행하며 비교 플래그를 2씩 곱함
				strcat(buf,"               "); //buf에 파일 간격 띄어쓰기 문자를 붙임
				if(compare_flag&flag){//매개 변수 플래그와 비교 플래그를 and연산하여 참이면
					strcat(buf," "); //공백을 붙임
					continue;
				}
				strcat(buf,"|");//|를 붙임
				
			}
			printf("%s\n",buf);
			printf("%s\n",buf);
			printf("%s",buf);//buf를 3번 출력
		}else {
			if(phase == 1){//함수가 첫번째 수행이면
				printf("----------------");//---------------- 출력
				for(j=0;j<blank;j++)//디렉터리 명 만큼 - 제거
					printf("\b");
			}
			else
				for(j=0;j<15-strlen(cPath);j++)//첫번째 수행이 아니면 15-디렉터리 글자수 만큼 - 출력
					printf("-");
			
		}
		

		if(S_ISDIR(statbuf.st_mode)){ //파일이 디렉터리면
			printf("\b");
			printf("|-%s",dirp->d_name); //파일을 출력하고
			if(i == count-1){ //이 파일이 마지막 파일이면
				flag += pow(2,phase-1); //해당 디렉터리 단계의 플래그를 1로 변경
			}
			print_tree(dirp->d_name,direc_filter,phase,blank,flag); //트리 출력함수 실행
			continue;
		}
		printf("\b");
		printf("|-%s\n",dirp->d_name); //파일 출력
		free(namelist[i]);
	}
	free(namelist);
	chdir(cwd);	
}