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
#include <sys/wait.h>
#include <signal.h>
#include "ssu_mntr.h"

pid_t main_pid;

int ssu_mntr(){
	char command[BUFLEN];
	char command_token[MAXARG][BUFLEN];
	char path[BUFLEN];
	size_t str_size;
	int flag;
	struct stat stbuf;

	getcwd(path,BUFLEN); //현재 작업디렉터리를 가져옴
	ssu_daemon(path); //디몬 프로세스 실행
	while(1){
		flag =0; //리턴 플래그를 0으로 설정
		init(command_token); //command_token 배열 초기화
		signal(SIGUSR1, delete_signal); //SIGUSR1에 delete_signal 등록
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

//command_token 변수 초기화
void init(char (*command_token)[BUFLEN]){
	int i;
	main_pid = getpid();
	for(i=0;i<MAXARG;i++)
		strcpy(command_token[i],"");
}

//사용자의 명령을 띄어쓰기 단위로 쪼개주는 함수
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
	if(!(strcmp(command_token[0],"delete"))){
		delete_command(command_token); //첫번째가 delete 면 delete 명령수행
		return 1;
	}
	else if(!(strcmp(command_token[0],"size"))){
		size_command(command_token);//첫번째가 size 면 size 명령수행
		return 1;
	}
	else if(!(strcmp(command_token[0],"recover"))){
		recover_command(command_token);//첫번째가 recover 면 recover 명령수행
		return 1;
	}
	else if(!(strcmp(command_token[0],"tree"))){
		tree_command(command_token);//첫번째가 tree 면 tree 명령수행
		return 1;
	}
	else if(!(strcmp(command_token[0],"exit"))){
		printf("Exit Program\n");//첫번째가 exit 면 exit 명령수행
		return -1;
	}
	else if(!(strcmp(command_token[0],"help"))){
		help_commnad();//첫번째가 help 면 help 명령수행
		return 1;
	}
	else if(!(strcmp(command_token[0],""))){
		return 1;//아무것도 입력하지 않으면 다음 프롬프트 출력
	}
	else{
		help_commnad();//그외에 다른 값이 입력되면 help 명령 수행
		return 1;
	}
}

//사용법을 출력하는 함수
int help_commnad(){
	printf("DELETE [FILENAME] [END_TIME] [OPTION]   :	Delete file commnad\n");
	printf("SIZE [FILENAME] [OPTION]                  :   Print directory size\n");
	printf("RECOVER [FILENAME] [OPTION]              :   Recover file command\n");
	printf("TREE                                         :   Print directory tree\n");
	printf("EXIT                                         :   Exit program\n");
	printf("HELP                                         :   Print usage\n");
}


//SIGUSR1을 받아 명령을 처리하는 함수
void delete_signal(int signo){
	int status;
	pid_t pid;
	pid = wait(&status); //자식 프로세스가 끝날때 까지 대기
	return;
}



