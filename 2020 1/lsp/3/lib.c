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
#include "crontab.h"

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

long long ipow(int x, int y){
	int i;
	long long ret = 1;
	for(i = 0;i<y;i++){ //x를 y번만큼 곱함
		ret *= x;
	}
	return ret;
}

int write_log(FILE * fp,char *cmd ,char (*command)[BUFLEN], char (*arg)[BUFLEN]){
	int i;
	time_t t;
	char buf[BUFLEN+1], *stime;

	fseek(fp, 0, SEEK_END);
	time(&t);
	stime = ctime(&t);
	snprintf(buf,26,"[%s",stime);
	strcat(buf,"] ");
	fwrite(buf,strlen(buf),1,fp); //시간정보를 기입

	sprintf(buf, "%s ",cmd);
	fwrite(buf,strlen(buf),1,fp); //명령 형태 기입(run,add 등)

	//인자의 내용을 순차적으로 기입
	for(i = 0;i<ADDCOMMAND;i++){
		sprintf(buf, "%s ",command[i]);
		fwrite(buf,strlen(buf),1,fp);
	}

	for(i = 0;i<MAXARG-ADDCOMMAND-1;i++){
		if(!strcmp(arg[i],""))
			break;
		sprintf(buf, "%s ",arg[i]);
		fwrite(buf,strlen(buf),1,fp);
	}
	sprintf(buf,"\n");
	fwrite(buf,1,1,fp);
	return 0;
}	

int print_cmd_list(FILE * fp){ //파일의 명령을 내용을 출력하는 함수

	int i = 0;
	char buf[BUFLEN];
	fseek(fp, 0, SEEK_SET);
	while(fgets(buf, BUFLEN, fp) != NULL){
		printf("%d. %s",i++,buf);
	}

	return 0;
}

int Addparse_command(char (*command)[BUFLEN], struct data * _data){
	char *ptr, *prev_ptr;
	char buf[3];
	int i, flag,tmp;
	long long listflag;

	for(i = 0; i < ADDCOMMAND;i++){ //각 주기에 대한 반복 수행
		ptr = command[i];
		prev_ptr = command[i];
		listflag  = 0;
		while(*ptr != '\0'){ //문자에 대한 반복수행
			switch(*ptr){
				case ',': //문자가 ,인 경우
					memset(buf,0,3);
					strncpy(buf, prev_ptr, ptr-prev_ptr); //buf에 ,이전 내용 복사
					if(*(prev_ptr-1) == '-'){ //이전에 -가 나온경우
						addcommand_addflag(_data,i,flag, uatoi(buf),'-');//addcommand_addflag 수행
					}
					else if(*(prev_ptr-1) == '/'){//이전 문자가 /인경우
						if(listflag != 0){ // - 주기 형식에 대한 플래그가 0이 아닌경우
							addcommand_addflag(_data,i,listflag, uatoi(buf),'/');//addcommand_addflag 수행
							listflag = 0;
						}
						else{
							addcommand_addflag(_data,i,-1, uatoi(buf),'/');//addcommand_addflag 수행
						}
					}
					else{
						flag = uatoi(buf); //buf를 정수로 변경
						addcommand_addflag(_data,i,uatoi(buf), -1,-1);//addcommand_addflag 수행
					}
					prev_ptr = ptr + 1;//prev_ptr을 ptr+1로 변경
					break;
				case '-'://문자가 -인 경우
					memset(buf,0,3);
					strncpy(buf, prev_ptr, ptr-prev_ptr);//buf에 ,이전 내용 복사
					prev_ptr = ptr + 1;//prev_ptr을 ptr+1로 변경
					flag = uatoi(buf);//buf를 정수로 변경
					break;
				case '/'://문자가 /인 경우
					memset(buf,0,3);
					tmp = flag;
					strncpy(buf, prev_ptr, ptr-prev_ptr);
					flag = uatoi(buf);
					if(*(prev_ptr-1) == '-')//이전에 -가 나온경우
						if((listflag = getlistflag(i,tmp,flag)) < 0) return -1; //getlistflag 수행 및 listflag 값 변경

					prev_ptr = ptr + 1;//prev_ptr을 ptr+1로 변경
					break;
				default :
					if(!(isdigit(*ptr) || *ptr == '*'))//문자가 숫자 또는 *이 아닌경우 오류
						return -1;
					break;
			}
			++ptr;
			if(*ptr == '\0'){ //문자열이 끝난 경우
				memset(buf,0,3);
				strncpy(buf, prev_ptr, ptr-prev_ptr);//문자 전의 문자열을 buf에 복사
				if(*(prev_ptr-1) == '-'){//이전에 -가 나온경우
					addcommand_addflag(_data,i,flag, uatoi(buf),'-');//addcommand_addflag 수행
				}
				else if(*(prev_ptr-1) == '/'){//이전 문자가 /인경우
					if(listflag != 0){// - 주기 형식에 대한 플래그가 0이 아닌경우
						addcommand_addflag(_data,i,listflag, uatoi(buf),'/');//addcommand_addflag 수행

					}
					else{
						addcommand_addflag(_data,i,-1, uatoi(buf),'/');//addcommand_addflag 수행
						
					}
				}
				else{
					flag = uatoi(buf);
					addcommand_addflag(_data,i,uatoi(buf), -1,-1);	//addcommand_addflag 수행
				}
			}
		}
	}
}

int addcommand_addflag(struct data * _data,int command,long long a1, int a2,char flag){
	int i;
	long long addFlag = 0, compare=1;
	int maxcycle[ADDCOMMAND] = {59,23,31,12,6};
	int mincycle[ADDCOMMAND] = {0,0,1,1,0};

	if(((a1 < mincycle[command] && a1 != -1) || (a2 > maxcycle[command] && a2 != -1))&&flag != '/') //주기 정보를 파악하여 입력이 잘못된 경우 -1 리턴
		return -1;

	switch(flag){ //flag 형식 판별(-,/에 대한 구분)
		case -1://flag -1인 경우
			if(a1 == -1){ //*이 입력된 경우
				//플래그를 전부 켬
				compare = ipow(2,mincycle[command]);
				for(i=mincycle[command];i<=maxcycle[command];i++,compare*=2){
					addFlag |= compare;
				}
				
			}
			else{//a1의 숫자에 맞게 플래그를 켬
				addFlag |= ipow(2, a1);
			}
			break;
		case '-': //- 인경우
			if((addFlag = getlistflag(command,a1,a2)) < 0){//getlistflag 을 수행하여 플래그 정보를 가져옴
				return -1;
			}
			break;
		case '/':// /인 경우
			if(a1 == -1){//앞의 인자가 * 이면
				for(i=mincycle[command];i<=maxcycle[command];i++,compare *= 2){ //주기의 실행횟수에 맞게 플래그를 켬
					if((i-mincycle[command]) % a2 == 0){
						
						addFlag |= compare;
					}
				}
			}
			else{//앞에 목록이 등장한 경우
				int j = 0;
				for(i=mincycle[command];i<=maxcycle[command];i++,compare *= 2){//목록의 플래그에 맞게 주기 실행횟수를 계산해 플래그를 켬
					if(a1 & compare){
						if(j % a2 == 0){
							addFlag |= compare;
						}
						j++;
					}
				}
			}
			break;
		default :
			break;
	}

	

	switch(command){//설정하고 싶은 주기의 종류에 대해 플레그를 셋팅
		case 0 :
			_data->minute |= addFlag;
			break;
		case 1 :
			_data->hour |= addFlag;
			break;
		case 2 :
			_data->day |= addFlag;
			break;
		case 3 :
			_data->month |= addFlag;
			break;
		case 4 :
			_data->DoW |= addFlag;
			break;
	}
}

long long getlistflag(int command,int a1, int a2){
	int i;
	long long addFlag = 0, compare=1;
	int maxcycle[ADDCOMMAND] = {59,23,31,12,6};
	int mincycle[ADDCOMMAND] = {0,0,1,1,0};

	if(a1 < mincycle[command] || a2 > maxcycle[command]) //주기가 최솟값 최댓값 사이에 있지 않다면 오류 리턴
		return -1;

	compare = ipow(2,a1); //목록에 맞게 플래그를 세팅
	for(i=0;i<a2-a1+1;i++,compare *= 2){
		addFlag |= compare;
	}
	return addFlag;
}