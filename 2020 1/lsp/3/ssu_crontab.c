#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "crontab.h"


int ssu_crontab(void){
	char command[BUFLEN];
	char command_token[MAXARG][BUFLEN];
	char path[BUFLEN];
	size_t str_size;
	int flag;
	struct stat stbuf;

	getcwd(path,BUFLEN); //현재 작업디렉터리를 가져옴
	while(1){
		flag =0; //리턴 플래그를 0으로 설정
		init(command_token); //command_token 배열 초기화
		printf("%s>",STUID);//프롬프트 출력
		fflush(stdout); //stdout 버퍼 비움
		chdir(path); //현재 작업디렉터리로 작업디렉터리 변경
		str_size = read(STDIN_FILENO,command,sizeof(command)); //사용자 명령어 읽어들임
		command[str_size-1] = '\0'; //명령의 마지막 개행을 널로 변경
		check_option(command,command_token); //옵션 tokenize 실행
		flag=runCommand(command_token); //옵션 실행
		if(flag <0){ //리턴 플래그가 0보다 작으면 프로그램 종료
			return 0;
		}
		memset(command, 0 ,sizeof(command)); //커맨드 변수 초기화
		chdir(path); //현재 작업디렉터리로 작업디렉터리 변경
	}
}

void init(char (*command_token)[BUFLEN]){
	int i;
	for(i=0;i<MAXARG;i++)
		strcpy(command_token[i],"");
}

int check_option(char * command,char (*command_token)[BUFLEN]){
	int i = 0;
	char * ptr = command; //command의 첫번째 포인터를 ptr에 대입
	strcpy(command_token[i],ptr); //command_token의 첫번째에 command 복사
	ptr = strtok(ptr," "); //공백단위로 쪼갬
	while(ptr != NULL){
		strcpy(command_token[i],ptr); //command[i]에 공백단위로 쪼갠 명령을 대입
		ptr = strtok(NULL," ");
		i++;
	}
}

int runCommand(char (*command_token)[BUFLEN]){
	if(!(strcmp(command_token[0],"add"))){
		add_command(command_token); //첫번째가 delete 면 delete 명령수행
		return 1;
	}
	else if(!(strcmp(command_token[0],"remove"))){
		remove_command(command_token);//첫번째가 size 면 size 명령수행
		return 1;
	}
	else if(!(strcmp(command_token[0],"exit"))){
		return -1;
	}
}

