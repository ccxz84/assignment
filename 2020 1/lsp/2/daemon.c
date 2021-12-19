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
#include <signal.h>
#include "ssu_mntr.h"


FILE * fp;


int ssu_daemon(){

	struct tm * tm;
	struct dirent *dirp,**filelist;
	int  count,i;
	pid_t pid;
	int fd, maxfd;
	char * _realpath;
	struct file * head = (struct file *)malloc(sizeof(struct file)); //링크드 리스트의 헤드 생성
	//헤드 초기화
	strcpy(head->filename,""); 
	head->next = NULL;
	head->dir = NULL;

	if((pid = fork())<0){ //자식프로세스 생성
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if(pid != 0){ //부모프로세스 반환
		return 0;
	}

	//디몬 코딩 규칙
	pid = getpid();
	setsid();
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	maxfd = getdtablesize();

	for(fd = 0; fd < maxfd; fd++){
		close(fd);
	}

	umask(0);
	fd = open("/dev/null",O_RDWR);
	dup(0);
	dup(0);
	//디몬 코딩 규칙

	if(access(DIRECTORY, F_OK) <0){
		if(mkdir(DIRECTORY,0755) < 0){
			return -1;
		}
	}
	

	fp = fopen("log.txt","a+"); //로그파일을 열기
	
	setbuf(fp,NULL); //버퍼를 사용하지 않음
	_realpath = realpath(DIRECTORY,NULL); //모니터링 경로를 절대경로로 변경
	chdir(_realpath); //모니터링 경로로 작업디렉터리 이동
	count = scandir(_realpath,&filelist,direc_filter,alphasort); //scandir 수행
	if(daemon_init(filelist,count,head) <0){ //링크드 리스트 초기화
		return -1;
	}
	while(1){ //무한반복
		count = scandir(_realpath,&filelist,direc_filter,alphasort);//scandir 수행
		creat_file(filelist,count,head); //생성파일 체크
		delete_file(filelist,count,head); //제거파일 체크
		modified_file(filelist,count,head); //수정파일 체크
		daemon_init(filelist,count,head); //링크드 리스트 초기화
		sleep(1); //1초 대기
	}

	fclose(fp); //로그 파일 닫음
	return 0;
}

void creat_file(struct dirent  **filelist, int count,struct file * head){
	struct file * cur;
	struct dirent * dirp;
	int i,flag,dcount;
	time_t ctime;
	struct tm * tm;
	struct dirent ** dir_filelist;
	struct stat statbuf;
	char buf[BUFLEN];

	for(i=0;i<count;i++){ //filelist에 대한 반복 수행
		dirp = filelist[i];
		flag = 0; //flag를 0으로 초기화
		cur = (head != NULL ? head->next : NULL); //head가 널이면 cur에 null 아니면 head->next 대입
		
		if(lstat(dirp->d_name,&statbuf) < 0){ //파일의 stat 구조체를 구함
			continue;
		}
		while(cur != NULL){ //cur에 대해 링크드리스트 탐색
			if(!(strcmp(dirp->d_name,cur->filename))){ //filelist의 파일명과 링크드 리스트의 파일명이 같으면
				flag = 1;
				if(S_ISDIR(statbuf.st_mode)&&cur->dir != NULL){ //파일이 디렉터리이면서 리스트에 저장되어 있다면
					dcount = scandir(dirp->d_name,&dir_filelist,direc_filter,alphasort);
					chdir(cur->filename);
					creat_file(dir_filelist,dcount,cur->dir); //디렉터리에 대해 생성파일 체크
					chdir("..");
				}
				break;
			}
			cur= cur->next; 
		}

		if(flag == 0){ //flag 가 0일때
			if(S_ISDIR(statbuf.st_mode)){ //디렉터리면
				dcount = scandir(dirp->d_name,&dir_filelist,direc_filter,alphasort);
				chdir(dirp->d_name);
				creat_file(dir_filelist,dcount,NULL); //생성된 파일을 모두 파악하기 위해 생성파일 체크 함수 실행
				chdir("..");
			}
			time(&ctime);
			tm = localtime(&ctime);
			strftime(buf,BUFLEN,"%Y-%m-%d %H:%M:%S",tm);
			fprintf(fp,"[%s][create_%s]\n",buf,dirp->d_name); //로그파일에 내용 출력
		}
	}
}

void delete_file(struct dirent  **filelist, int count,struct file * head){
	struct file * cur = head->next, *dcur;
	struct dirent * dirp, ** dir_filelist;
	int i,flag,dcount;
	time_t ctime;
	struct tm * tm;
	char buf[BUFLEN];
	char cwd[FILELEN];
	getcwd(cwd,FILELEN);

	while(cur != NULL){ //리스트에 대해 반복 수행
		flag = 0; //flag를 0으로 초기화
		if(cur->dir != NULL){ //파일이 디렉터리면 
			dcount = scandir(cur->filename,&dir_filelist,direc_filter,alphasort);
			chdir(cur->filename);
			delete_file(dir_filelist,dcount,cur->dir); //삭제 파일 체크 함수 실행
			chdir("..");
		}
			
		for(i = 0;i<count;i++){ //filelist에 대해 반복 수행
			dirp = filelist[i];
			if(!(strcmp(cur->filename,dirp->d_name))){ //리스트의 파일명과 링크드 리스트의 파일명이 같으면 
				flag = 1;//flag를 1로 변경
				break;

			}
		}
		if(flag == 0){ //flag가 0이면
			time(&ctime);
			tm = localtime(&ctime);
			strftime(buf,BUFLEN,"%Y-%m-%d %H:%M:%S",tm);
			fprintf(fp,"[%s][delete_%s]\n",buf,cur->filename); //로그파일에 내용출력
		}
		cur = cur->next;
	}
	chdir(cwd);
}


void modified_file(struct dirent  **filelist, int count,struct file * head){
	struct file * cur = head->next;
	struct dirent * dirp, **dir_filelist;
	struct stat statbuf;
	int i,dcount;
	time_t ctime;
	struct tm * tm;
	char buf[BUFLEN];

	for(i=0;i<count;i++){ //filelist에 대해 반복 수행
		dirp = filelist[i];
		cur = head->next;
		while(cur != NULL){ //링크드 리스트에 대해 반복 수행
			if(!(strcmp(dirp->d_name,cur->filename))){ //리스트의 파일명과 링크드 리스트의 파일명이 같을때
				if(stat(dirp->d_name,&statbuf) < 0){ //stat구조체를 가져옴
					break;
				}
				if(S_ISDIR(statbuf.st_mode)&&cur->dir != NULL){ //파일이 디렉터리면
					dcount = scandir(dirp->d_name,&dir_filelist,direc_filter,alphasort);
					chdir(cur->filename);
					modified_file(dir_filelist,dcount,cur->dir); //수정파일 탐색함수 수행
					chdir("..");
				}
				if(cur->mtime != statbuf.st_mtime){ //파일의 수정시간이 링크드 리스트에 저장된 수정시간과 다르면
					time(&ctime);
					tm = localtime(&ctime);
					strftime(buf,BUFLEN,"%Y-%m-%d %H:%M:%S",tm);
					fprintf(fp,"[%s][modify_%s]\n",buf,cur->filename);//로그에 내용출력
				}
				break;
			}
			cur= cur->next;
		}
	}

}

int daemon_init(struct dirent ** filelist,int count,struct file * head){
	int i,dcount;
	struct dirent *dirp;
	struct file * cur = head;
	struct stat statbuf;
	struct dirent ** dir_filelist;
	
	if(head->next != NULL){
		free_list(head); //기존의 링크드 리스트 초기화
	}
	
	for(i=0;i<count;i++){ //filelist에 대해 반복수행
		cur->next = (struct file *)malloc(sizeof(struct file)); //새로운 노드 생성
		cur = cur->next;
		strcpy(cur->filename,filelist[i]->d_name);//노드에 파일명 복사
		if(lstat(cur->filename,&statbuf) < 0){
			continue;
		}
		cur->next = NULL;
		cur->dir = NULL;//next 노드 dir 노드 널로 초기화
		cur->mtime = statbuf.st_mtime; //수정 시간 초기화
		if(S_ISDIR(statbuf.st_mode)){ //파일이 디렉터리면
			dcount = scandir(cur->filename,&dir_filelist,direc_filter,alphasort);
			chdir(cur->filename);
			cur->dir = (struct file *)malloc(sizeof(struct file));//dir 노드 생성
			strcpy(cur->dir->filename,cur->filename);
			cur->dir->next = NULL;
			cur->dir->dir = NULL;
			daemon_init(dir_filelist,dcount,cur->dir); //디렉터리 안의 파일에 대해 리스트 생성
			chdir("..");
		}	
		free(filelist[i]);
	}
	free(filelist);
}

void free_list(struct file * head){

	struct file * cur;

	while(head->next != NULL){//리스트에 대해 반복수행
		cur = head->next;
		if(cur->dir != NULL){ //dir이 널이 아니면
			free_list(cur->dir);
			free(cur->dir); //cur->dir 을 헤드로 하여 리스트 삭제
		}
		head->next = cur->next;
		free(cur); //노드 삭제
	}
}