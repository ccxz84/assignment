#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include "crontab.h"

int remove_command(char (*command_token)[BUFLEN]){
	int num = atoi(command_token[1]), i;
	FILE * file_fp, *log_fp;
	size_t size1, size2, full_size;
	char buf[BUFLEN], *ptr, *stime;
	time_t t;

	time(&t);
	stime = ctime(&t);
	stime[strlen(stime)-1] = 0;
	if(access("ssu_crontab_file", F_OK) <0){
		log_fp = fopen("ssu_crontab_file", "w+");
		fclose(log_fp);
	}
	 //로그파일을 열기

	if(access("ssu_crontab_log", F_OK) <0){
		file_fp = fopen("ssu_crontab_log", "w+");
		fclose(file_fp);
	}
	file_fp = fopen("ssu_crontab_file","r+"); //로그파일을 열기
	log_fp = fopen("ssu_crontab_log","r+");

	for(i=0;i<num;i++){
		fgets(buf,BUFLEN,file_fp);
	}
	size1 = ftell(file_fp); //지울 문자열의 맨 앞 offset
	fgets(buf,BUFLEN,file_fp);
	size2 = ftell(file_fp);//지울 문자열의 맨 뒤 offset
	fseek(file_fp,0,SEEK_END);
	full_size = ftell(file_fp); //파일의 전체 크기
	fseek(file_fp, size2, SEEK_SET);

	ptr = malloc(full_size - size2); //지울 문자열 뒷 부분의 문자열을 담을 변수 생성
	fread(ptr,full_size-size2,1,file_fp); //문자열 읽어옴
	fseek(file_fp,size1,SEEK_SET);
	fwrite(ptr,full_size-size2,1,file_fp); //지울 문자열의 맨 앞에 overwrite

	fseek(file_fp, 0, SEEK_SET);
	ftruncate(fileno(file_fp),full_size - (size2 - size1)); //나머지 부분을 삭제

	fseek(log_fp,0,SEEK_END);

	fprintf(log_fp, "[%s] remove %s",stime,buf);
	
	fflush(file_fp);
	print_cmd_list(file_fp);
	fclose(file_fp);
	fclose(log_fp);
}