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
#include <signal.h>
#include "ssu_mntr.h"

extern pid_t main_pid;

int delete_command(char (*command_token)[BUFLEN]){
	char buf[BUFLEN];
	char * filename, *tmp;
	char * path;
	struct tm s_tm;
	time_t rmtime,ctime;
	pid_t pid;
	struct stat statbuf;
	char yn;

	char _day[BUFLEN],_time[BUFLEN];

	if(trashDirectory_check() < 0){ //trash 디렉터리가 있는 지확인
		fprintf(stderr, "trash directory error\n");
		return -1;
	}
	if((path = realpath(command_token[1],NULL)) == NULL){ //사용자가 입력한 경로를 절대경로로 변경
		fprintf(stderr, "\'%s\' File not found\n",command_token[1]);
		return -1;
	}
	//buf를 이용해 경로에서 파일명만 추출
	strcpy(buf,path);
	tmp = buf;
	while(tmp < buf + strlen(buf)){
		if(*tmp == '/')
			filename = tmp+1;
		tmp++;
	}

	if(strcmp(command_token[2],"")&&strcmp(command_token[3],"")){//사용자가 삭제 예약 시간을 입력했다면
		strcpy(_day,command_token[2]);
		strcpy(_time,command_token[3]);
		strcat(_time,":00");
		if(get_time_struct(_day,_time,&s_tm) == NULL){ //사용자의 시간 문자열을 struct tm 구조체로 변경
			fprintf(stderr, "day, time input error\n");
			return -1;
		}
		rmtime = mktime(&s_tm); //struct tm 구조체를 time_t 로 변경
		if((pid = fork()) < 0){ //자식프로세스를 생성하고
			fprintf(stderr, "fork error\n");
			return -1;
		}
		else if(pid > 0){ //부모프로세스를 리턴함
			return 0;
		}
		else if(pid == 0){ //자식프로세스는 
			time(&ctime);
			sleep(rmtime-ctime); //예약시간 - 현재시간 만큼 대기
			if(!(strcmp(command_token[4],"-r"))||!(strcmp(command_token[5],"-r"))){ //r 옵션이 입력된 경우
				kill(main_pid,SIGUSR1); //부모프로세스에게 대기 요청을 보냄
				printf("\nDelete [y/n]?  "); //사용자에게 삭제 여부를
				fflush(stdout);
				fflush(stdin);
				scanf("%c",&yn);
				switch(yn){ //y면 삭제, n면 자식프로세스 종료
					case 'y':
						break;
					case 'n':
						exit(0);
					default:
						break;
				}
			}
			if(!(strcmp(command_token[4],"-i"))||!(strcmp(command_token[5],"-i"))){ //i옵션이 입력된 경우
				lstat(path,&statbuf);
				if(S_ISDIR(statbuf.st_mode)){ //파일이 디렉터리면
					delete_directory(path); //디렉터리 제거함수 실행
				}
				remove(path); //파일 제거
			}
			else{//i옵션이 없다면
				if(make_info(path,filename) < 0){//info 파일 생성
					exit(0);
				}
				sprintf(buf,"%s/%s/%s",TRASH,FILES,filename);
				rename(path,buf);//files 폴더로 파일 이동
			}
			exit(0);
		}

	}

	if(!(strcmp(command_token[2],"-i"))){//i옵션이 입력된 경우
		lstat(path,&statbuf);
		if(S_ISDIR(statbuf.st_mode)){//파일이 디렉터리면
			delete_directory(path);//디렉터리 제거함수 실행
		}
		remove(path);//파일 제거
	}
	else{//i옵션이 없다면
		if(make_info(path,filename) < 0){//info 파일 생성
			return -1;
		}
		sprintf(buf,"%s/%s/%s",TRASH,FILES,filename);
		rename(path,buf);//files 폴더로 파일 이동
	}
}

int trashDirectory_check(){
	char cwd[FILELEN];
	getcwd(cwd,FILELEN);
	if(access(TRASH, F_OK) <0){//trash 폴더가 있는지 체크하고 없다면 생성
		if(mkdir(TRASH,0777) < 0){
			return -1;
		}
	}
	chdir(TRASH);
	if(access(INFO,F_OK) < 0){ //info 폴더가 있는지 체크하고 없다면 생성
		if(mkdir(INFO,0777) < 0){
			chdir(cwd);
			return -1;
		}
	}
	if(access(FILES,F_OK) < 0){//files 폴더가 있는지 체크하고 없다면 생성
		if(mkdir(FILES,0777) < 0){
			chdir(cwd);
			return -1;
		}
	}
	chdir(cwd);
	return 0;
}

int make_info(char * path, char * filename){
	char buf[BUFLEN]= ""	;
	int fd;
	time_t ctime;
	struct stat statbuf;
	struct tm * tm;
	char cwd[FILELEN];

	getcwd(cwd,FILELEN);//현재의 작업디렉터리 백업
	sprintf(buf,"%s/%s",TRASH,INFO);
	off_t size =get_directory_size(buf,direc_filter); //info 폴더의 디렉터리 사이즈 체크
	while(size > MAXINFOSIZE){ //최대 사이즈 보다 크다면
		delete_info(buf); //info 파일을 오래된 순으로제거
		size =get_directory_size(buf,direc_filter);//info 폴더의 디렉터리 사이즈 체크
	}
	if((lstat(path,&statbuf))< 0){ //삭제할 파일의 stat 구조체를 가져옴 없다면 에러
		fprintf(stderr, "%s stat error\n",path);
		chdir(cwd);
		return -1;
	}

	chdir(TRASH);
	chdir(INFO);//trash/info 로 작업 디렉터리 변경
	time(&ctime); //현재 시간을 구함
	strcpy(buf, filename); 
	strcat(buf,".txt"); //파일명에 .txt를 붙임

	if(access(buf,F_OK) >= 0){
		get_info_num(buf); //만약 파일이 중복되면 중복을 피하기 위한 번호를 가져옴
	}
	if((fd =open(buf,O_CREAT|O_TRUNC|O_WRONLY,0666)) <0 ){ //파일 생성
		fprintf(stderr,"%s open error\n",buf);
		chdir(cwd);
		return -1;
	}
	strcpy(filename,buf);//filename에 수정된 파일명 복사

	write(fd,"[Trash info]\n",strlen("[Trash info]\n"));
	write(fd,path,strlen(path));
	write(fd,"\n",1);

	tm = localtime(&ctime);
	strftime(buf,BUFLEN,"%Y-%m-%d %H:%M:%S\n",tm);


	write(fd,"D : ",strlen("D : "));
	write(fd, buf, strlen(buf));

	tm = localtime(&statbuf.st_mtime);
	strftime(buf,BUFLEN,"%Y-%m-%d %H:%M:%S\n",tm);

	write(fd,"M : ",strlen("M : "));
	write(fd, buf, strlen(buf));
	sprintf(buf,"%s/%s",TRASH,INFO);
	//파일절대 경로 및 필요한 데이터를 파일에 씀.
	size =get_directory_size(buf,direc_filter); //info 폴더의 디렉터리 사이즈 체크
	while(size > MAXINFOSIZE){ //최대 사이즈 보다 크다면
		delete_info(buf); //info 파일을 오래된 순으로제거
		size =get_directory_size(buf,direc_filter);//info 폴더의 디렉터리 사이즈 체크
	}
	chdir(cwd); //작업디렉터리 복원
	return 0;
}

int delete_info(char * dPath){
	DIR * dp;
	time_t mtime ;
	struct stat statbuf;
	struct dirent *dirp,*rdirp;
	char buf[FILELEN];

	getcwd(buf,FILELEN); //현재 작업디렉터리 백업

	time(&mtime);

	if((dp = opendir(dPath)) == NULL){
		fprintf(stderr, "opendir error\n");
		chdir(buf);
		return -1;
	}
	chdir(dPath); //매개변수의 디렉터리로 작업디렉터리 변경
	//디렉터리를 순회하며 수정시간이 가장오래된 파일을 체크
	while((dirp = readdir(dp)) != NULL){
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) 
			continue;
		lstat(dirp->d_name,&statbuf);
		if(statbuf.st_mtime < mtime){
			mtime = statbuf.st_mtime;
			rdirp = dirp;
		}
	}

	lstat(rdirp->d_name,&statbuf); //파일의 stat구조체를 가져옴
	if(S_ISDIR(statbuf.st_mode)){ //폴더면 폴더 전체 삭제
		delete_directory(rdirp->d_name);
	}
	remove(rdirp->d_name); //파일 삭제
	chdir(buf);
	chdir(TRASH);
	chdir(FILES);//trash/files 경로로 이동
	lstat(rdirp->d_name,&statbuf); //파일의 stat구조체를 가져옴
	if(S_ISDIR(statbuf.st_mode)){ //폴더면 폴더 전체 삭제
		delete_directory(rdirp->d_name);
	}
	remove(rdirp->d_name); //파일 삭제
	chdir(buf); //현재 작업디렉터리 복원
}

char * get_info_num(char * buf){
	int id = 1;
	char fbuf[BUFLEN];
	strcpy(fbuf,buf);
	while(access(fbuf,F_OK)>=0){
		sprintf(fbuf,"%d_%s",id,buf); //파일명에 숫자_파일명 을 붙이며 중복되는지 체크
		++id;
	}
	strcpy(buf,fbuf); 
	return buf;//중복되지 않은 파일명 반환
}



