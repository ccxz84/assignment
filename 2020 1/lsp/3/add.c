#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include "crontab.h"

int add_command(char (*command_token)[BUFLEN]){
	char command[ADDCOMMAND][BUFLEN];
	char arg[MAXARG-ADDCOMMAND-1][BUFLEN];
	struct data _data;
	int i,j;
	FILE * file_fp, * log_fp;

	memset(arg,0,(MAXARG-ADDCOMMAND-1)*BUFLEN); //arg 메모리 초기화
	for(i=1;i<ADDCOMMAND+1;i++){ //command_token 에서 주기 부분을 command에 복사
		if(!strcmp(command_token[i], ""))
			return -1;
		strcpy(command[i-1],command_token[i]);
	}

	if(valid_cycle(command) < 0){ //valid_cycle 수행
		return -1;
	}

	if(access("ssu_crontab_file",F_OK) < 0){//ssu_crontab_file 파일이 존재하는지 확인하고 존재하지 않으면 생성
		file_fp = fopen("ssu_crontab_file", "w+");
		fclose(file_fp);
	}
	file_fp = fopen("ssu_crontab_file","r+");//ssu_crontab_file 열기
	for(j= 0;i < MAXARG;i++,j++){//arg에 실행할 명령 내용을 복사
		if(!strcmp(command_token[i], ""))
			break;
		strcpy(arg[j],command_token[i]);
	}
	add_file(file_fp, command, arg); //명령 내용을 파일에 추가
	fflush(file_fp); //파일 버퍼를 비움
	if(access("ssu_crontab_log",F_OK) < 0){	 //로그 파일이 있는지 검사하고 파일이 존재하지 않으면 생성
		log_fp = fopen("ssu_crontab_log", "w+");
		fclose(log_fp);
	}
	log_fp = fopen("ssu_crontab_log","r+"); //로그 파일을 읽기 전용으로 열기
	write_log(log_fp,command_token[0], command, arg); //로그 쓰기
	fflush(log_fp);
	print_cmd_list(file_fp); //파일의 내용을 터미널에 출력

	fclose(file_fp);
	fclose(log_fp);
}

int valid_cycle(char (*command)[BUFLEN]){
	char *ptr, *prev_ptr;
	char buf[3];
	int i, flag, tmp;
	int maxcycle[ADDCOMMAND] = {59,23,31,12,6};
	int mincycle[ADDCOMMAND] = {0,0,1,1,0};

	for(i= 0;i<ADDCOMMAND;i++){ //주기 내용을 개수 만큼 반복
		ptr = command[i];
		prev_ptr = command[i];
		while(*ptr != '\0'){ //널문자를 만날때까지 반복
			switch(*ptr){
				case ',': //문자가 , 인경우
					memset(buf,0,3); //buf 초기화
					strncpy(buf, prev_ptr, ptr-prev_ptr); //문자 전의 문자열을 buf에 복사
					if(!strcmp(buf, "*")){ //문자가 *인 경우 다음 반복 진행
						prev_ptr = ptr + 1;
						break;
					}
					if(uatoi(buf) < 0) //buf를 정수로 변경
						return -1;
					if(*(prev_ptr-1) == '-'){ //전 문자열중 - 이 있는경우
						if(flag > uatoi(buf)) //전에 등장한 숫자가 지금 등장한 숫자보다 큰 경우 잘못되었으므로 -1 리턴
							return -1;
						if(flag < mincycle[i] || uatoi(buf) > maxcycle[i]) //숫자의 목록이 주기 범위에 들어가는지 판단
							return -1;
						prev_ptr = ptr + 1;
						break;
					}
					prev_ptr = ptr + 1;
					break;
				case '-'://문자가 - 인경우
					memset(buf,0,3); //buf 초기화
					strncpy(buf, prev_ptr, ptr-prev_ptr);//문자 전의 문자열을 buf에 복사
					if(!strcmp(buf, "*")){//문자가 *인 경우 다음 반복 진행
						prev_ptr = ptr + 1;
						break;
					}
					flag = uatoi(buf);//buf를 정수로 변경
					if(flag < 0) //숫자가 아니 거나 음수인 경우 -1 리턴
						return -1;
					if(flag < mincycle[i] || flag > maxcycle[i]) //주기 범위 파악
						return -1;
					prev_ptr = ptr + 1;
					break;
				case '/'://문자가 / 인경우
					memset(buf,0,3);//buf 초기화
					strncpy(buf, prev_ptr, ptr-prev_ptr);//문자 전의 문자열을 buf에 복사
					if(!strcmp(buf, "*")){//문자가 *인 경우 다음 반복 진행
						prev_ptr = ptr + 1;
						break;
					}
					if(*(prev_ptr-1) == '-'){//전에 문자가 - 인경우
						if(flag > uatoi(buf))//숫자로 변경했을 때 전의 숫자보다 작으면 오류
							return -1;
						if(flag < mincycle[i] || uatoi(buf) > maxcycle[i]) //주기범위 파악
							return -1;
						prev_ptr = ptr + 1;
						break;
					}
					flag = uatoi(buf); //flag에 숫자를 대입
					prev_ptr = ptr + 1;
					if(flag < mincycle[i] || flag > maxcycle[i]) //주기범위 파악
							return -1;
					break;
				default :
					if(!(isdigit(*ptr) || *ptr == '*')) //문자가 숫자 또는 *이 아닌경우 오류
						return -1;
					break;
			}
			++ptr;
			if(*ptr == '\0'){ //문자열이 끝난 경우
				memset(buf,0,3); //buf 초기화
				strncpy(buf, prev_ptr, ptr-prev_ptr);//문자 전의 문자열을 buf에 복사
				if(!strcmp(buf, "*")){//문자가 *인 경우 다음 반복 진행
					break;
				}
				if(*(prev_ptr-1) == '-'){//전에 문자가 - 인경우
					if(flag > uatoi(buf))//숫자로 변경했을 때 전의 숫자보다 작으면 오류
						return -1;
					if(flag < mincycle[i] || uatoi(buf) > maxcycle[i])//주기범위 파악
							return -1;
				}
				else{
					flag = uatoi(buf);//flag에 숫자를 대입
					if(flag < mincycle[i] || flag > maxcycle[i])//주기범위 파악
						return -1;
				}
			}
		}
	}
	return 0;
}

int add_file(FILE * fp,char (*command)[BUFLEN], char (*arg)[BUFLEN]){
	int i;
	char buf[BUFLEN+1];

	fseek(fp, 0, SEEK_END);

	for(i = 0;i<ADDCOMMAND;i++){ //주기를 buf에 쓴다
		sprintf(buf, "%s ",command[i]);
		fwrite(buf,strlen(buf),1,fp);
	}

	for(i = 0;i<MAXARG-ADDCOMMAND-1;i++){ //명령을 buf에 쓴다
		if(!strcmp(arg[i],""))
			break;
		sprintf(buf, "%s ",arg[i]);
		fwrite(buf,strlen(buf),1,fp);
	}
	sprintf(buf,"\n");
	fwrite(buf,1,1,fp);
	return 0;
}	

