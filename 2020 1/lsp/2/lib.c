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
#include <ctype.h>
#include "ssu_mntr.h"


off_t get_directory_size(char const *path,int (*filter)(const struct dirent *)){
	char cwd[FILELEN];
	struct dirent ** namelist, *dirp;
	int i,count;
	off_t size = 0;
	struct stat statbuf;
	count = scandir(path,&namelist,filter,alphasort); //파일경로에 대해 scandir 수행
	getcwd(cwd,FILELEN); //현재 작업디렉터리 백업
	chdir(path); //path로 작업디렉터리 이동
	for(i=0;i<count;i++){
		dirp = namelist[i];
		if(lstat(dirp->d_name,&statbuf)){ //파일의 stat구조체를 가져옴
			fprintf(stderr,"stat error\n");
			break;
		}
		if(S_ISDIR(statbuf.st_mode)){ //파일이 디렉터리인지 체크
			size += get_directory_size(dirp->d_name,direc_filter); //디렉터리 체크 함수를 실행하고 결과를 size에 더함
			continue;
		} 
		size += statbuf.st_size; //파일의 size를 size 변수에 더함
		free(namelist[i]);
	}
	chdir(cwd);
	return size;
}

int direc_filter(const struct dirent * dirp){
	if(!(strcmp(dirp->d_name,"."))||!(strcmp(dirp->d_name,".."))){ //디렉터리가 ., .. 이면 제거
		return 0;
	}
}

struct tm  *get_time_struct(char * day, char * time, struct tm * _time){
	int i = 0;
	int t_day[3];
	int t_time[3];
	char test[BUFLEN];

	day = strtok(day,"-"); //일자 문자열을 -로 자름
	while(day != NULL){

		if((t_day[i] = uatoi(day)) < 0) //자른 문자열을 숫자로 변환하고 t_day 배열에 저장
			return NULL;

		day = strtok(NULL,"-");//일자 문자열을 -로 자름

		i++;
	}

	_time->tm_year = t_day[0]-1900; //struct tm 구조체에 입력년도 - 1900 을 저장
	_time->tm_mon = t_day[1] - 1; //struct tm 구조체에 입력월 -1 을 저장
	_time->tm_mday = t_day[2]; //struct tm 구조체에 입력날을 저장

	i = 0;
	time = strtok(time,":"); //시간 문자열을 : 단위로 자름
	while(time != NULL){
		if((t_time[i] = uatoi(time)) < 0) //자른 시간 문자열을 숫자로 변환
			return NULL;
		time = strtok(NULL,":");
		i++;
	}
	
	_time->tm_hour = t_time[0]; //struct tm 구조체에 시간을 저장
	_time->tm_min = t_time[1]; //struct tm 구조체에 분을 저장
	_time->tm_sec = t_time[2]; //struct tm 구조체에 초를 저장

	//printf("%d-%d-%d %d:%d:%d\n",_time->tm_year ,_time->tm_mon,_time->tm_mday,_time->tm_hour,_time->tm_min,_time->tm_sec); //시간 파싱 체크를 위한 코드(주석처리)

	return _time;
}

int uatoi(char s[]) {
  int i, n, sign = -1;
  for (i = 0; isspace(s[i]); i++) //문자열의 공백을 넘어감
    ; /* skip white space */
  sign = (s[i] == '-') ? -1 : 1; //-가 있으면 음수로 변경
  if (s[i] == '+' || s[i] == '-') /* skip sign */ //+, - 부호를 넘어감
    i++;
  if(!isdigit(s[i])) //시간체크를 위해 다음 문자열이 숫자가 아닌경우 -1 리턴
  	return -1;
  for (n = 0; isdigit(s[i]); i++) n = 10 * n + (s[i] - '0'); //문자가 숫자인경우 n에 자릿수에 맞는 숫자를 더함
  return sign * n;
}

double pow(double x, double y){
	int i;
	double ret = 1;
	for(i = 0;i<y;i++){ //x를 y번만큼 곱함
		ret *= x;
	}
	return ret;
}

int delete_directory(char * path){
	struct dirent ** namelist;
	char * cwd;
	int count, i;
	struct stat statbuf;

	cwd = getcwd(NULL, FILELEN); //현재 디렉터리를 백업
	count = scandir(path,&namelist,direc_filter,alphasort);	//scandir 수행
	chdir(path);//path로 작업디렉터리 이동

	for(i=0;i<count;i++){ //filelist에 대해 반복
		if(lstat(namelist[i]->d_name,&statbuf) < 0){ //파일의 stat 구조체를 가져옴
			fprintf(stderr,"%s stat error\n",namelist[i]->d_name);
			continue;
		}

		if(S_ISDIR(statbuf.st_mode)){ //파일이 디렉터리라면
			delete_directory(namelist[i]->d_name); //디렉터리 제거함수 수행
		}
		remove(namelist[i]->d_name); //파일 제거
		free(namelist[i]);
	}
	free(namelist);
	chdir(cwd);
	free(cwd);
}