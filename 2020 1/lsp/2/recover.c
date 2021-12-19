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
#include <ctype.h>
#include "ssu_mntr.h"

int recover_command(char (*command_token)[BUFLEN]){

	int ret,num;
	char cwd[FILELEN];
	char buf[FILELEN];
	char * path;
	int numlist[MAXINFOFILE];
	FILE * fp;

	getcwd(cwd,FILELEN);

	if(trashDirectory_check() < 0){ //trash 디렉터리 체크
		fprintf(stderr, "trash directory error\n");
		chdir(cwd);
		return -1;
	}
	chdir(TRASH);
	chdir(INFO);//info 디렉터리로 작업디렉터리 변경
	if(!(strcmp(command_token[2],"-l"))){ //l옵션 확인
		print_oldlist(); //오래된 순으로 info 리스트 출력
	}

	if((ret = search_file(command_token[1],numlist)) > 0){ //사용자가 입력한 파일과 매칭되는 info 파일 탐색 및 정보 출력
		printf("Choose : ");
		scanf("%d",&num); //사용자에게 제거 파일을 입력받음
		
		if(num <= 0);
		else{
			num = numlist[num-1]; //제거 리스트에서 num을 초기화
			if(num == 0)
				sprintf(buf,"%s", command_token[1]); //num 이 0이면 중복처리한 파일을 복원하는 것이 아니므로 그냥 삭제
			else
				sprintf(buf,"%d_%s",num, command_token[1]); //0이 아니면 중복처리 된 파일을 복원하는 것이므로 파일명 변경
		}
		
	}
	else if(ret == 0)
		strcpy(buf,command_token[1]);//함수의 리턴값이 아니면 그냥 파일 복원
	else if(ret < 0){
		printf("Threr is no \'%s\' in the 'trash' directory\n",command_token[1]); //파일이 없으면 에러 출력
		chdir(cwd);
		return -1;
	}
	strcat(buf,".txt");
	strcpy(command_token[1],buf); //파일명 끝에 .txt를 붙임
	if((fp =fopen(buf,"r")) == NULL){
		fprintf(stderr,"%s file doesn't exist\n",buf);
		chdir(cwd);
		return -1;
	}
	fgets(buf,FILELEN,fp);
	fgets(buf,FILELEN,fp);//info 파일에서 복원 경로를 불러옴
	fclose(fp);
	unlink(command_token[1]); //info 파일 삭제
	chdir("..");
	chdir(FILES);
	path = realpath(command_token[1],NULL); //복원 파일의 절대경로를 구함
	buf[strlen(buf)-1] = '\0';
	rename(path,buf); //파일 복원
	
	chdir(cwd);
	return 0;

}

int search_file(char * filename,int * numlist){

	int id = 0,count,i,j,flag;
	FILE * fp;
	char buf[FILELEN];
	char infoname[FILELEN];
	char file[FILELEN];
	char num[FILELEN];
	struct dirent **namelist, *dirp;
	char * ptr;
	
	count = scandir(getcwd(buf,FILELEN),&namelist,direc_filter,alphasort); //scandir 수행
	strcpy(infoname,filename);
	strcat(infoname,".txt");//파일명에 .txt를 붙임

	for(i=0;i<count;i++){//파일에 대해 반복 수행
		dirp = namelist[i];
		flag = 0;
		
		strcpy(file, dirp->d_name); //파일명을 file 변수에 복사
		strcpy(num,"");
		if((fp =fopen(file,"r")) == NULL){ //파일을 오픈함
			fprintf(stderr,"%s fopen error\n",file);
			continue;
		}
		
		for(j = 0;j<strlen(file);j++){
			if(flag == 0 && isdigit(file[j])){ //파일명 맨 앞의 숫자를 가져옴
				num[j] = file[j];
				if(file[j+1] == '_'){
					num[j] = file[j];
					++j;
					flag = 1;
				}
				continue;
			}
			ptr = &file[j]; //숫자_파일명 구조에서 파일명을 ptr에 넣음
			if(!(strcmp(ptr,infoname))){ //파일명이 같은지 비교
				numlist[id] = atoi(num); //중복처리 번호를 배열에 넣는다
				++id;
				printf("%d. %s",id,filename);
				fgets(buf,FILELEN,fp);
				fgets(buf,FILELEN,fp);
				fgets(buf,FILELEN,fp);
				buf[strlen(buf)-1] = '\0';
				printf(" %s",buf);
				fgets(buf,FILELEN,fp);
				printf(" %s",buf); //필요한 정보를 출력
				fclose(fp);
				break;
			}
			else{
				fclose(fp);
				break;
			}
		}
		free(namelist[i]);
	}
	free(namelist);
	return id;
}

int print_oldlist(){
	
	int count,i,j;
	struct dirent ** namelist, *temp;
	time_t a1,a2;
	char buf[BUFLEN],*ptr;
	int flag;
	FILE * fp;

	count = scandir(getcwd(buf,FILELEN),&namelist,direc_filter,alphasort);//scandir 수행

	for(i=0;i<count-1;i++){ //filelist에 대해 버블 정렬 수행
		flag = 0;
		for(j=0;j<count-i-1;j++){
			a1 = get_delete_time(namelist[j]->d_name);//삭제 시간을 가져옴
			a2 = get_delete_time(namelist[j+1]->d_name);
			if(a1 > a2){ //삭제시간을 확인하여 조건을 만족하면 배열위치를 변경
				flag = 1;
				temp = namelist[j];
				namelist[j]=namelist[j+1];
				namelist[j+1]=temp;
			}
			
		}
		if(flag==0) break;
	}

	for(i=0;i<count;i++){ //filelist에 대해 반복수행
		if((fp =fopen(namelist[i]->d_name,"r")) == NULL){
			fprintf(stderr,"tt\n");
			continue;
		}
		fgets(buf,FILELEN,fp);
		fgets(buf,FILELEN,fp);
		fgets(buf,FILELEN,fp);
		buf[strlen(buf)-1] = '\0';
		ptr = &buf[4];		
		printf("%d. %s	%s\n",i+1,namelist[i]->d_name,ptr);//오래된 순으로 파일 리스트 출력
		fclose(fp);
		free(namelist[i]);
	}
	free(namelist);
	printf("\n\n");
}

time_t get_delete_time(char * filename){
	time_t ret;
	FILE * fp;
	char buf[BUFLEN], *day, *time;
	struct tm tm;

	if((fp =fopen(filename,"r")) == NULL){ //파일을 열기
		fprintf(stderr,"tt\n");
		return -1;
	}
	fgets(buf,FILELEN,fp);
	fgets(buf,FILELEN,fp);
	fgets(buf,FILELEN,fp);
	buf[strlen(buf)-1] = '\0';
	day = &buf[4];
	buf[14] = '\0';
	time = &buf[15];
	buf[23] = '\0'; //삭제된 시간 정보를 추출
	get_time_struct(day,time,&tm); //struct tm 구조체로 변경
	ret = mktime(&tm); //time_t 로 변경
	fclose(fp);
	return ret;
}