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
#include "ssu_mntr.h"

int size_command(char (*command_token)[BUFLEN]){
	struct stat statbuf;
	char buf[BUFLEN];
	int count;

	off_t size;

	if(lstat(command_token[1],&statbuf) < 0){ 
		fprintf(stderr,"stat error\n");
		return -1;
	}

	strcpy(buf,"./");
	strncat(buf,command_token[1],strlen(command_token[1]));
	if(S_ISDIR(statbuf.st_mode)){

		size = get_directory_size(command_token[1],direc_filter); //입력받은 경로의 디렉터리 사이즈를 체크
	}
	else{
		size = statbuf.st_size;//디렉터리가 아니면 파일의 사이즈를 대입
	}
	printf("%ld 	%s\n",size,buf);//파일 경로와, 사이즈를 출력

	if(!(strcmp(command_token[2], "-d"))){ //d 옵션이 입력되었다면
		count = atoi(command_token[3]);//하위 단계를 체크한다 
		search_directory(command_token[1],1,count);//디렉터리 검색함수 실행
	}
}

int search_directory(char * path, int ccount,int count){

	struct dirent ** namelist, *dirp;
	int filecount,i;
	off_t size;
	struct stat statbuf;
	char buf[BUFLEN];
	char cwd[BUFLEN];

	getcwd(cwd,BUFLEN); //현재 작업디렉터리 복사

	filecount = scandir(path,&namelist,direc_filter,alphasort); //scandir 실행

	chdir(path); //입력된 디렉터리로 작업 디렉터리 변경

	if(ccount >= count){ //현재 단계와 사용자가 입력한 단계를 비교하여 작은 경우 중단
		chdir(cwd);
		return 1;
	}

	for(i=0;i<filecount;i++){ //디렉터리안의 파일에 대해 반복
		dirp = namelist[i];

		if(lstat(dirp->d_name,&statbuf) < 0){ //stat 구조체를 가져옴
			fprintf(stderr,"%s/%s error\n",path,dirp->d_name);
			free(namelist[i]);
			continue;
		}

		if(S_ISDIR(statbuf.st_mode)){ //디렉터리라면
			sprintf(buf,"%s/%s",path,dirp->d_name); //path에 현재 디렉터리/탐색중인 디렉터리를 넣는다.
			size = get_directory_size(dirp->d_name,direc_filter); //디렉터리 사이즈 검색
			chdir(cwd); //작업 디렉터리 변경
			search_directory(buf,ccount+1,count);//디렉터리 검색 수행
			chdir(path); //작업디렉터리 변경
		}
		else
			size = statbuf.st_size; //파일이면 stat 구조체의 사이즈 대입

		sprintf(buf,"./%s/%s",path,dirp->d_name); //경로 출력
		printf("%ld	%s\n",size,buf); //사이즈 출력
		free(namelist[i]); //dirent 변수 반환
	}
	free(namelist);
	chdir(cwd); //작업 디렉터리 변경
}