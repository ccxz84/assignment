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
#include "crontab.h"


FILE * log_fp;
FILE * file_fp;


int main(){

	int fd, maxfd;
	time_t t,t1;
	struct tm * _t;
	struct stat statbuf;
	pid_t pid;

	struct list * head = (struct list *)malloc(sizeof(struct list)); //링크드 리스트의 헤드 생성
	//헤드 초기화
	head->next = NULL;

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

	if(access("ssu_crontab_file", F_OK) <0){
		log_fp = fopen("ssu_crontab_file", "w+");
		fclose(log_fp);
	}
	 //로그파일을 열기

	if(access("ssu_crontab_log", F_OK) <0){
		file_fp = fopen("ssu_crontab_log", "w+");
		fclose(file_fp);
	}
	file_fp = fopen("ssu_crontab_file","r"); //로그파일을 열기
	log_fp = fopen("ssu_crontab_log","r+");
	
	setbuf(log_fp,NULL); //버퍼를 사용하지 않음

	
	if(daemon_init(file_fp,head) <0){ //링크드 리스트 초기화
		return -1;
	}
	lstat("ssu_crontab_file",&statbuf);
	t = statbuf.st_mtime;
	
	
	while(1){
		time(&t1);
		_t = localtime(&t1);
		if(_t->tm_sec == 0)
			break;
	}
	while(1){ //무한반복
		lstat("ssu_crontab_file",&statbuf);
		if(t != statbuf.st_mtime){ //명령파일이 변경된 경우
			daemon_init(file_fp,head);//daemon_init 수행
			t = statbuf.st_mtime;
		}
		run_command(log_fp,head); //run_command 수행
		sleep(60); //1초 대기
	}

	fclose(file_fp); //로그 파일 닫음
	fclose(log_fp);
	return 0;
}

void run_command(FILE * fp, struct list * head){

	struct list * cur = (head != NULL ? head->next : NULL);
	time_t t;
	struct tm * _t;
	int i,j,list[5];
	long long _list[5];
	long long compare = 1;
	char buf[BUFLEN], * stime;
	int ret;

	time(&t);
	_t = localtime(&t);//struct tm 구조체를 가져옴
	fseek(fp, 0, SEEK_END);

	stime = ctime(&t); //시간을 가져옴
	

	list[0] = _t->tm_min;
	list[1] = _t->tm_hour;
	list[2] = _t->tm_mday;
	list[3] = _t->tm_mon;
	list[4] = _t->tm_wday;

	for(i=0;i<5;i++){ //시간에 맞게 strut data 구조체의 시간 플래그를 설정
		compare = 1;
		for(j = 0;j < list[i];j++,compare *= 2);
		_list[i] = compare;
	}

	while(cur != NULL){ //링크드 리스트 순회

		if((ret = time_check(cur, _list)) == 0 && ret != -1){//time_check 수행 하여 시간이 맞아 명령이 수행되면
			snprintf(buf,26,"[%s",stime);
			strcat(buf,"] run ");
			strcat(buf, cur->_data.origin);
			fwrite(buf, strlen(buf),1, fp); //로그 찍기
		}
		cur = cur->next;
	}
}

int time_check(struct list * cur, long long list[]){
	int i;

	if(!((cur->_data.minute & list[0]) && (cur->_data.hour & list[1]) && (cur->_data.day & list[2]) && (cur->_data.month & list[3]) && (cur->_data.DoW & list[4]))){ //링크드 리스트의 시간과 현재 시간을 비교

		return -1;
	}
	return system(cur->_data.command); //맞다면 명령실행

}

int daemon_init(FILE * fp,struct list * head){
	
	char buf[BUFLEN], *ptr;
	char command[ADDCOMMAND][BUFLEN];
	int i;
	struct list *tmp;

	free_list(head);

	fseek(fp, 0, SEEK_SET);
	while(fgets(buf, BUFLEN,fp) != NULL){
		tmp = head->next;
		head->next = (struct list *)malloc(sizeof(struct list)); //head->next 설정
		head->next->next = tmp;
		i=0;
		strcpy(head->next->_data.origin,buf); //원래 텍스트 내용 대입
		ptr = buf;
		ptr = strtok(ptr," ");
		while(ptr != NULL){
			strcpy(command[i],ptr); //command[i]에 공백단위로 쪼갠 명령을 대입
			ptr = strtok(NULL," ");
			i++;
			if(i > 4)
				break;
		}
		if(Addparse_command(command,&(head->next->_data)) < 0){ //Addparse_command 를 수행
			tmp = head->next;
			head->next = head->next->next;
			free(tmp);
			continue;
		}
		strcpy(head->next->_data.command,ptr); //명령 내용 대입
	}
	return 0;
}

void free_list(struct list * head){

	struct list * cur;

	while(head->next != NULL){//리스트에 대해 반복수행
		cur = head->next;
		head->next = cur->next;
		free(cur); //노드 삭제
	}
}