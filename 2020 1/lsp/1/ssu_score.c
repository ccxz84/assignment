#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "blank.h"

extern struct ssu_scoreTable score_table[QNUM];
extern char id_table[SNUM][10];

struct ssu_scoreTable score_table[QNUM];
char id_table[SNUM][10];

char stuDir[BUFLEN];
char ansDir[BUFLEN];
char errorDir[BUFLEN];
char threadFiles[ARGNUM][FILELEN];
char iIDs[ARGNUM][FILELEN];

int eOption = false;
int tOption = false;
int mOption = false; //m 옵션을 위한 변수
int iOption = false; //m 옵션을 위한 변수

void ssu_score(int argc, char *argv[])
{
	int fd;
	if((fd = open("/dev/null", O_WRONLY)) < 0){
		fprintf(stderr, "open error /dev/null\n");
		exit(1);
	}
	dup2(fd, 2);
	char saved_path[BUFLEN];
	int i;

	for(i = 0; i < argc; i++){ //h옵션을 받으면 print_usage 실행
		if(!strcmp(argv[i], "-h")){
			print_usage();
			return;
		}
	}

	memset(saved_path, 0, BUFLEN);
	if(argc >= 3 != 0){ //인자가 3개 이상이면 학생답 경로 정답 경로 복사
		strcpy(stuDir, argv[1]);
		strcpy(ansDir, argv[2]);
	}

	if(!check_option(argc, argv)) //check option 함수 실행
		exit(1);

	if(!eOption && !tOption && iOption && !mOption&&((opendir(stuDir) ==NULL)||(opendir(ansDir) ==NULL))){ //i 옵션만 받았다면 do_iOption 실행
		do_iOption(iIDs);
		return;
	}

	getcwd(saved_path, BUFLEN); //현재 작업경로 복사
	if(chdir(stuDir) < 0){ //학생답 경로로 작업경로 변경
		fprintf(stdout, "%s doesn't exist\n", stuDir);
		return;
	}
	getcwd(stuDir, BUFLEN); //stuDir에 현재 작업경로 복사

	chdir(saved_path); //처음 작업경로로 이동
	if(chdir(ansDir) < 0){ //정답 작업경로로 이동
		fprintf(stdout, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN); //정답 경로에 현재 작업경로 복사

	chdir(saved_path); //처음 작업경로로 이동

	set_scoreTable(ansDir); //set_scoreTable 실행

	set_idTable(stuDir); //set_idTable 실행

	printf("grading student's test papers..\n");
	score_students(); //score_students 실행
	if(iOption){
		do_iOption(iIDs); //i옵션을 받았다면 do_iOptions 실행
	}

	return;
}

int check_option(int argc, char *argv[])
{
	int i, j;
	int c;

	while((c = getopt(argc, argv, "e:thmie")) != -1) //옵션으로 thmie 를 받았는지 확인
	{
		switch(c){ //각 옵션별로 코드 수행
			case 'e':
				eOption = true;//e옵션 플래그 참으로 변경
				strcpy(errorDir, optarg); //errorDir에 e 옵션 인자 복사

				if(access(errorDir, F_OK) < 0) //errordir 이 있는지 확인
					mkdir(errorDir, 0755); //없으면 755권한으로 생성
				else{
					rmdirs(errorDir); //rmdirs 함수 실행
					mkdir(errorDir, 0755);//755 권한으로 디렉터리 생성
				}
				break;
			case 'm':
				mOption = true; //m옵션 플래그 참으로 변경
				break;
			case 'i':
				iOption = true;//i옵션 플래그 참으로 변경
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){ //i옵션 인자 반복

					if(j >= ARGNUM) //index가 5개 넘으면 에러메시지 출력
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(iIDs[j], argv[i]); //iIDs 에 학번 복사
					i++; 
					j++;
				}
				break;
			case 't':
				tOption = true; //t 옵션 플래그 참으로 변경
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){ //t 옵션 인자 반복

					if(j >= ARGNUM) //index가 5개를 넘으면 에러메시지 출력
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else //threadFiles에 문제번호 복사
						strcpy(threadFiles[j], argv[i]);
					i++; 
					j++;
				}
				break;

			case '?':
				printf("Unkown option %c\n", optopt); //이외의 옵션이 들어오면 에러메시지 출력
				return false;
		}
	}

	return true;
}

void do_iOption(char (*ids)[FILELEN])
{
	char tmp[BUFLEN];
	int idx;
	int i = 0;
	char *p;
	char qname[QNUM][FILELEN];
	
	FILE * fp;
	if((fp = fopen("score.csv", "r")) == NULL){ //score.csv를 읽기전용으로 열기
		fprintf(stdout, "file open error for score.csv\n");
		return;
	}

	fscanf(fp, "%s\n", tmp); //tmp에 fp에서 문자열 읽기
	idx = read_gradeTable(fp, qname, tmp); //read_gradeTable 함수 실행

	while(fscanf(fp, "%s\n", tmp) != EOF) //tmp에 fp에서 문자열 읽기
	{
		i = 0;
		p = strtok(tmp, ","); //, 를 기준으로 문자열 자르기
		

		if(!is_exist(ids, tmp)) //ids에 tmp의 학번 번호가 있는지 확인
			continue;

		printf("%s's wrong answer : \n", tmp); //tmp 학번의 틀린 답은

		while((p = strtok(NULL, ",")) != NULL){ //, 를 기준으로 각 문항의 점수 파악
			if(!strcmp(p, "0.00") && idx > i){ //각 항목이 0점인지 확인
				printf("%s",qname[i]); //0점이면 틀린것으로 출력
				printf(", ");
			}
			i++;
			
		}
		printf("\b\b \n");
	}
	fclose(fp);
}

int read_gradeTable(FILE * fp, char (*qname)[FILELEN] ,char tmp[BUFLEN]){
	char *str;
	int idx = 0;
	str = strtok(tmp, ",");
	for(;idx<QNUM;idx++){ //idx를 QNUM 만큼 반복
		
		if(!strcmp(str, "sum")){ //,를 기준으로 자른 문자가 sum이면 끝이므로 리턴
			return idx;
		}
		strcpy(qname[idx],str); //qname[idx]에 문제번호 복사
		str = strtok(NULL,","); //, 기준으로 문자열 자르기
	}
	return 0;
}

int is_exist(char (*src)[FILELEN], char *target)
{
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM) //i 값이 5가 넘는지 확인
			return false;
		else if(!strcmp(src[i], "")) //src[i]가 공백인지 확인
			return false;
		else if(!strcmp(src[i++], target)) //src[i]에 학번이 있으면 참 반환
			return true;
	}
	return false;
}

void set_scoreTable(char *ansDir)
{
	char filename[FILELEN];

	sprintf(filename, "%s/%s", ansDir, "score_table.csv");

	if(access(filename, F_OK) == 0){
		read_scoreTable(filename); //점수 테이블을 읽어들임
		if(mOption == true){
			edit_question_grade(); //m 옵션이 입력되면 점수 수정을 함
			write_scoreTable(filename); //점수 수정을 마친 후, 테이블을 최신화 함.
		}
	}
	else{
		make_scoreTable(ansDir); //점수테이블 만들기
		write_scoreTable(filename); //score_table.csv 쓰기
	}
}

void edit_question_grade(){
	char str[BUFLEN];
	char qname[FILELEN];
	double new_score;
	int i;
	int size = sizeof(score_table)/sizeof(score_table[0]); //score_table 의 갯수
	int qname_size; //문제번호의 문자열 길이
	while(1){
		qname_size = 0; 
		printf("Input question's number to modify : "); 
		scanf("%s",str); //유저에게 수정할 문제를 입력받음
		if(!strcmp(str, "no")){ //no 이면 종료
			break;
		}
		for(i = 0; i < size; i++){ //score_table의 size만큼 반복
			if(!strcmp(score_table[i].qname,"")){ //score_table의 qname이 공백이면 종료
				break;
			}
			strcpy(qname, score_table[i].qname); //score_table의 qname을 qname 변수에 복사
			qname_size = strlen(qname); //qname 변수의 문자열 길이
			strtok(qname, "."); //qname 을 . 기준으로 자르기
			
			if(!strcmp(qname, str)){ //자른 qname이 유저가 입력한 str과 같으면
				printf("Current score : %.2lf\nNew score : ",score_table[i].score); //현재 점수를 출력하고, 고칠 점수를 입력받음
				scanf("%lf",&new_score);
				score_table[i].score = new_score; //score_table의 score 값 변경
				break;
			}
			memset(qname, 0, qname_size); //qname 메모리 초기화
		}

	}
}

void read_scoreTable(char *path)
{
	FILE *fp;
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){ //path의 파일을 읽기전용으로 열기
		fprintf(stdout, "file open error for %s\n", path);
		return ;
	}

	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){ //fp로부터 문제번호와 점수 읽기
		strcpy(score_table[idx].qname, qname); //score_table의 qname에 문제번호 복사
		score_table[idx++].score = atof(score); //score_table의 score에 점수 복사
	}

	fclose(fp);
}

void make_scoreTable(char *ansDir)
{
	int type, num;
	double score, bscore, pscore;
	struct dirent *dirp, *c_dirp;
	DIR *dp, *c_dp;
	char tmp[BUFLEN];
	int idx = 0;
	int i;

	num = get_create_type(); //사용자가 어떤 입력 방식을 원하는지 확인

	if(num == 1)
	{
		printf("Input value of blank question : "); //블랭크 문제 점수 입력 받음
		scanf("%lf", &bscore);
		printf("Input value of program question : "); //프로그램 문제 점수 입력 받음
		scanf("%lf", &pscore);
	}
	
	if((dp = opendir(ansDir)) == NULL){ //정답 디렉터리 열기
		fprintf(stdout, "open dir error for %s\n", ansDir);
		return;
	}	

	while((dirp = readdir(dp)) != NULL) //dp에 대해 readdir 수행
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) //.이나 ..이면 다음 반복 수행
			continue;
		if((type = get_file_type(dirp->d_name)) < 0) //file의 확장자가 .c 나 .txt가 아니면 다음 반복 수행
			continue;

		strcpy(score_table[idx++].qname, dirp->d_name); //score_table의 qname에 파일명 복사

	}

	closedir(dp);
	sort_scoreTable(idx); //score_table 변수 정렬

	for(i = 0; i < idx; i++)
	{
		type = get_file_type(score_table[i].qname); //qname의 문제 타입 불러오기

		if(num == 1) //입력 타입이 1일때, 
		{
			if(type == TEXTFILE) //텍스트 파일이면 점수에 bscore 대입
				score = bscore;
			else if(type == CFILE) //c 파일이면 점수에 pscore 대입
				score = pscore;
		}
		else if(num == 2) //입력 타입이 2일때,
		{
			printf("Input of %s: ", score_table[i].qname); //각 문제의 점수 불러오기
			scanf("%lf", &score);
		}

		score_table[i].score = score; //score_table의 score의 문제 점수 대입
	}
}	

void write_scoreTable(char *filename)
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]); //score_table의 size

	if((fd = creat(filename, 0666)) < 0){ //filename의 경로의 파일을 생성
		fprintf(stdout, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++)
	{
		if(score_table[i].score == 0) //score_table의 score가 0이면 종료
 			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score);//tmp에 '문제 번호, 점수'를 쓰기
		write(fd, tmp, strlen(tmp)); //파일에 tmp를 write
	}

	close(fd);
}


void set_idTable(char *stuDir)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

	if((dp = opendir(stuDir)) == NULL){ //학생 답 디렉터리를 open
		fprintf(stdout, "opendir error for %s\n", stuDir);
		exit(1);
	}

	while((dirp = readdir(dp)) != NULL){ //디렉터리의 파일 불러오기
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))//.이나 ..이면 다음 반복 수행
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name); //tmp에 '학생답/학번' 쓰기
		stat(tmp, &statbuf); //statbuf의 tmp 경로의 stat 구조체 불러오기

		if(S_ISDIR(statbuf.st_mode)) //tmp 경로의 파일이 디렉터리인지 확인
			strcpy(id_table[num++], dirp->d_name); //id_table의 값에 학생답 폴더의 학번 복사
		else
			continue;
	}

	sort_idTable(num); //id_table 변수 정렬
}

void sort_idTable(int size) //id_table을 정렬
{
	int i, j;
	char tmp[10];

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 -i; j++){
			if(strcmp(id_table[j], id_table[j+1]) > 0){
				strcpy(tmp, id_table[j]);
				strcpy(id_table[j], id_table[j+1]);
				strcpy(id_table[j+1], tmp);
			}
		}
	}
}

void sort_scoreTable(int size)
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 - i; j++){

			get_qname_number(score_table[j].qname, &num1_1, &num1_2); //score_table의 qname을 변수로 주어 -을 제외한 번호를 추출
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2);


			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){ //문제번호의 앞이 비교대상과 크거나 같으면 그리고 문제번호의 뒤가 비교대상보다 크다면

				memcpy(&tmp, &score_table[j], sizeof(score_table[0])); //i, j+1의 score_table 값 변경
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			}
		}
	}
}

void get_qname_number(char *qname, int *num1, int *num2)
{
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname)); //dup에 qname 복사
	*num1 = atoi(strtok(dup, "-."));//.,- 기준으로 문자열을 자르고 num1에 대입
	
	p = strtok(NULL, "-."); //.,- 기준으로 문자열을 자른다
	if(p == NULL)
		*num2 = 0;
	else
		*num2 = atoi(p); //p의 값에 따라 num2에 대입
}

int get_create_type() //사용자에게 입력 타입을 받는 함수
{
	int num;

	while(1)
	{
		printf("score_table.csv file doesn't exist in TREUDIR!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num);

		if(num != 1 && num != 2)
			printf("not correct number!\n");
		else
			break;
	}

	return num;
}

void score_students()
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]); //score_table 의 size

	if((fd = creat("score.csv", 0666)) < 0){ //score.csv 오픈
		fprintf(stdout, "creat error for score.csv");
		return;
	}
	write_first_row(fd); //score.csv의 첫줄을 입력하는 함수

	for(num = 0; num < size; num++) //num을 size만큼 반복
	{
		if(!strcmp(id_table[num], "")) //id_table[num] 이 공백이면 종료
			break;

		sprintf(tmp, "%s,", id_table[num]); //tmp에 '학번,' 입력
		write(fd, tmp, strlen(tmp));  //score.csv에 tmp 입력

		score += score_student(fd, id_table[num]); //score_student의 리턴값을 score에 더한다
	}
	printf("Total average : %.2f\n", score / num); // 총점의 평균 출력

	close(fd);
}

double score_student(int fd, char *id)
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);

	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0) //score_table의 score 값이 0 이면 종료
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname); //tmp에 학생답경로/학번/문제번호 를 쓴다

		if(access(tmp, F_OK) < 0){ //tmp 파일이 있는지 확인 없으면 false 반환
			result = false;
		}
		else
		{
			if((type = get_file_type(score_table[i].qname)) < 0) //score_table의 qname 파일 타입 가져오기 
				continue;
			
			if(type == TEXTFILE){//.txt 이면 score_blank 실행
				result = score_blank(id, score_table[i].qname);
			}
			else if(type == CFILE){ //.c 이면 score_program 실행
				result = score_program(id, score_table[i].qname);
			}
		}

		if(result == true){ //결과 값이 참이면
			score += score_table[i].score; //score에 score_table의 score 더한다
			sprintf(tmp, "%.2f,", score_table[i].score); //tmp에 score_table의 score 쓰기
			write(fd, tmp, strlen(tmp)); //fd에 tmp를 write
		}
		else if(result < 0){ //결과가 0보다 작으면
			score = score + score_table[i].score + result; //score + score_table의 score + result를 더한다
			sprintf(tmp, "%.2f,", score_table[i].score + result); //tmp에 score_table의 score + result를 쓴다
			write(fd, tmp, strlen(tmp)); //fd에 tmp 를 쓴다
		}
		else{
			score += result; //score 에 result 더한다
			sprintf(tmp, "%.2f,", result); //tmp에 result를 쓴다
			write(fd, tmp, strlen(tmp)); //fd에 tmp를 write
		}
	}

	printf("%s is finished.. score : %.2f\n", id, score); 

	sprintf(tmp, "%.2f\n", score); //tmp에 score를 쓴다
	write(fd, tmp, strlen(tmp)); //fd에 tmp를 write

	return score;
}

void write_first_row(int fd)
{
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); //score_table의 size

	write(fd, ",", 1); //fd에 ,를 write 함

	for(i = 0; i < size; i++){
		if(score_table[i].score == 0) //score_table의 score 가 0 이면 종료
			break;
		
		sprintf(tmp, "%s,", score_table[i].qname); //'문제번호,'를 tmp에 쓴다
		write(fd, tmp, strlen(tmp)); //fd에 tmp를 write
	}
	write(fd, "sum\n", 4);//fd에 'sum\n'를 쓴다
}

char *get_answer(int fd, char *result)
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN); //result 메모리를 0으로 초기화
	while(read(fd, &c, 1) > 0) //fd에서 문자를 하나 읽어옴
	{
		if(c == ':') //문자가 : 이면 종료
			break;
		
		result[idx++] = c; //result에 문자를 쓴다
	}
	if(result[strlen(result) - 1] == '\n') //result의 마지막이 개행이면 널로 변경
		result[strlen(result) - 1] = '\0';

	return result;
}

int score_blank(char *id, char *filename)
{
	char tokens[TOKEN_CNT][MINLEN];
	node *std_root = NULL, *ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN];
	int fd_std, fd_ans;
	int result = true;
	int has_semicolon = false;

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename); //학생답경로/학번/문제번호 를 tmp에 쓴다
	fd_std = open(tmp, O_RDONLY); //tmp를 읽기전용으로 열기
	strcpy(s_answer, get_answer(fd_std, s_answer)); //get_answer의 반환값을 s_answer에 복사

	if(!strcmp(s_answer, "")){ //s_answer에 아무것도 없다면 종료
		close(fd_std);
		return false;
	}

	if(!check_brackets(s_answer)){ //괄호수가 맞는지 확인
		close(fd_std);
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer))); //s_answer의 좌우 공백을 제거해 s_answer에 복사

	if(s_answer[strlen(s_answer) - 1] == ';'){ //s_answer의 마지막이 ; 이면 
		has_semicolon = true; //has_semicolon 값을 참으로 변경
		s_answer[strlen(s_answer) - 1] = '\0'; //마지막 문자를 \0로 변경
	}

	if(!make_tokens(s_answer, tokens)){ //s_answer를 tokenize 함
		close(fd_std);
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0); //token으로 연산 트리를 만든다

	sprintf(tmp, "%s/%s", ansDir, filename); //정답경로/문제번호 를 tmp에 쓴다
	fd_ans = open(tmp, O_RDONLY); //정답 문제파일을 읽기전용으로 열기

	while(1)
	{
		ans_root = NULL; //정답트리 head
		result = true; //결과값

		for(idx = 0; idx < TOKEN_CNT; idx++) //TOKEN_CNT 만큼 반복
			memset(tokens[idx], 0, sizeof(tokens[idx])); //tokens 배열의 idx 번을 0으로 초기화
		strcpy(a_answer, get_answer(fd_ans, a_answer)); //a_answer에 정답을 가져옴

		if(!strcmp(a_answer, "")) //정답에 아무것도 없다면 종료
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer))); //a_answer의 좌우 공백을 제거하고 a_answer에 복사

		if(has_semicolon == false){ //has_semicolon이 거짓일때
			if(a_answer[strlen(a_answer) -1] == ';') //a_answer의 마지막 값이 ; 이면 다음 반복 진행
				continue;
		}

		else if(has_semicolon == true) //has_semicolon이 참일때
		{
			if(a_answer[strlen(a_answer) - 1] != ';') //a_answer의 마지막이 ; 이 아니라면 다음 반복진행
				continue;
			else
				a_answer[strlen(a_answer) - 1] = '\0'; //참이면 a_answer의 마지막을 \0으로 변경
		}

		if(!make_tokens(a_answer, tokens)) //a_answer의 tokenize가 거짓이면 다음 반복 수행
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0); //정답의 연산 트리를 만든다

		compare_tree(std_root, ans_root, &result); //학생답과 정답의 트리를 비교

		if(result == true){ //결과값이 참이면
			close(fd_std);
			close(fd_ans); //학생답과 정답 파일을 닫는다

			if(std_root != NULL) //학생트리가 있으면 트리를 비운다
				free_node(std_root);
			if(ans_root != NULL) //정답트리가 있다면 트리를 비운다
				free_node(ans_root);
			return true;

		}
	}
	
	close(fd_std);
	close(fd_ans);

	if(std_root != NULL) //학생트리가 있으면 트리를 비운다
		free_node(std_root);
	if(ans_root != NULL) //정답트리가 있다면 트리를 비운다
		free_node(ans_root);
	return false;
}

double score_program(char *id, char *filename)
{
	double compile;
	double result;
	compile = compile_program(id, filename); //filename의 .c 파일을 컴파일 함

	if(compile == false || compile == ERROR){ //컴파일 값이 false 거나 ERROR 이면 에러값을 반환
		return compile;
	}
	
	result = execute_program(id, filename); //filename의 프로그램을 실행

	if(!result){ //결과가 0이 아니면
		return false; //거짓 반환
	}

	if(compile < 0){ //컴파일이 0보다 작으면
		return compile; //컴파일 값 반환
	}
	return result; //결과 값 반환
}

int is_thread(char *qname)
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]); //threadFiles의 size

	for(i = 0; i < size; i++){ //size 만큼 반복
		if(!strcmp(threadFiles[i], qname)) //문제번호가 threadFiles에 해당하면
			return true; //참 반환
	}
	return false;
}

double compile_program(char *id, char *filename)
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN];
	int isthread;
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname)); //qname 0으로 초기화
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //qname에 filename의 문자열을 filename에서 .이 등장하기 전까지의 크기로 복사
	
	isthread = is_thread(qname); //is_thread 수행

	sprintf(tmp_f, "%s/%s", ansDir, filename); //tmp_f에 정답경로/filename 을 쓴다
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname); //tmp_e에 정답경로/문제번호.exe 를 쓴다

	if(tOption && isthread) //tOption이 켜져 있고, isthread 가 참이라면
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f); //command에 'gcc -o 정답경로/문제번호.exe 정답경로/filename -lpthread'를 쓴다
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);//command에 'gcc -o 정답경로/문제번호.exe 정답경로/filename'를 쓴다

	sprintf(tmp_e, "%s/%s_error.txt", ansDir, qname); //tmp_e에 정답경로/문제번호_error.txt 를 쓴다
	fd = creat(tmp_e, 0666); //tmp_e 경로의 파일을 0666 권한으로 생성

	redirection(command, fd, STDERR); //redirection 함수 실행
	size = lseek(fd, 0, SEEK_END); //fd 파일의 size 파악
	close(fd);
	unlink(tmp_e); //tmp_e 파일 unlink

	if(size > 0) // 컴파일 에러가 났다면 거짓 반환
		return false;

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename); //학생답경로/학번/문제번호 를 tmp_f에 쓴다
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname); //학생답경로/학번/문제번호.stdexe 를 tmp_e를 쓴다

	if(tOption && isthread) //tOption이 켜져있고, isthread가 참이라면
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f); //command에 'gcc -o 학생답경로/학번/문제번호.stdexe 학생답경로/학번/문제번호 -lpthread'를 쓴다
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f); //command에 'gcc -o 학생답경로/학번/문제번호.stdexe 학생답경로/학번/문제번호'를 쓴다

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname); //tmp_f에 학생답경로/학번/문제번호_error.txt 를 쓴다
	fd = creat(tmp_f, 0666); //tmp_f를 0666 권한으로 생성한다

	redirection(command, fd, STDERR); //redirection 함수를 실행
	size = lseek(fd, 0, SEEK_END); //fd의 크기를 체크
	close(fd);

	if(size > 0){ //크기가 0보다 크다면
		if(eOption) //eOption이 켜져 있다면
		{
			sprintf(tmp_e, "%s/%s", errorDir, id); //tmp_e에 에러파일경로/학번 을 쓴다
			if(access(tmp_e, F_OK) < 0) //에러파일경로/학번이 없다면
				mkdir(tmp_e, 0755); //디렉터를 755권한으로 만든다

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname); //에러파일경로/학번/문제번호_error.txt 를 tmp_e에 쓴다
			rename(tmp_f, tmp_e); //tmp_f 파일을 tmp_e 로 rename 한다

			result = check_error_warning(tmp_e); //check_error_warning의 결과를 result에 대입
		}
		else{ 
			result = check_error_warning(tmp_f); //check_error_warning의 결과를 result에 대입
			unlink(tmp_f); //tmp_f 파일을 unlink
		}

		return result; //result를 반환
	}

	unlink(tmp_f); //tmp_f 파일을 unlink
	return true; 
}

double check_error_warning(char *filename)
{
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;

	if((fp = fopen(filename, "r")) == NULL){ //filename을 읽기전용으로 열기
		fprintf(stdout, "fopen error for %s\n", filename);
		return false;
	}

	while(fscanf(fp, "%s", tmp) > 0){ //tmp에 문자열을 읽어들임
		if(!strcmp(tmp, "error:")){ //tmp가 error: 라면
			return ERROR; //에러 반환
		}
		else if(!strcmp(tmp, "warning:")) //tmp가 warnin: 이라면
			warning += WARNING; //워닝 추가
	}

	return warning; //워닝 반환
}

double execute_program(char *id, char *filename)
{
	char std_fname[BUFLEN], ans_fname[BUFLEN];
	char tmp[BUFLEN];
	char qname[FILELEN];
	time_t start, end;
	pid_t pid;
	int fd;

	memset(qname, 0, sizeof(qname)); //qname 0으로 초기화
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //filename에서 .을 제외한 문제번호만 qname 에 복사

	sprintf(ans_fname, "%s/%s.stdout", ansDir, qname); //ans_fname에 정답경로/문제번호.stdexe 를 복사
	fd = creat(ans_fname, 0666); //ans_fname 을 0666 권한으로 생성

	sprintf(tmp, "%s/%s.exe", ansDir, qname);//tmp에 정답경로/문제번호.exe 를 쓴다
	redirection(tmp, fd, STDOUT); //redirection 실행
	close(fd);

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname); //std_fname에 학생답경로/학번/문제번호.stdout 을 쓴다
	fd = creat(std_fname, 0666); //std_fname 을 0666 권한으로 생성

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname); //tmp에 학생답경로/학번/문제번호.stdexe 를 쓴다

	start = time(NULL); //현재 시간을 가져옴
	redirection(tmp, fd, STDOUT); //redirection을 실행
	
	sprintf(tmp, "%s.stdexe", qname); //tmp에 문제번호.stdexe 를 쓴다
	while((pid = inBackground(tmp)) > 0){ //pid에 프로세스 pid 를 가져온다
		end = time(NULL); //현재시간을 end에 가져옴

		if(difftime(end, start) > OVER){ //end와 start를 비교해서 OVER보다 크다면
			kill(pid, SIGKILL); //프로세스에 kill 시그널을 보낸다
			close(fd); //파일을 닫고 
			return OVER_SCORE; //시간초과 점수를 리턴한다
		}
	}

	close(fd);

	return compare_resultfile(std_fname, ans_fname); //결과파일 비교 함수를 실행한다
}

pid_t inBackground(char *name)
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;
	
	memset(tmp, 0, sizeof(tmp));
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); //background.txt 를 읽기 쓰기 전용으로 생성하고 기존 내용은 제거한다

	sprintf(command, "ps | grep %s", name); //ps | grep 문제번호.stdexe 를 command에 쓴다
	redirection(command, fd, STDOUT); //redirection 실행

	lseek(fd, 0, SEEK_SET); //fd 파일의 offset을 0으로 변경
	read(fd, tmp, sizeof(tmp)); //tmp의 사이즈 만큼 fd에서 읽어들인다

	if(!strcmp(tmp, "")){ //tmp에 아무것도 없다면
		unlink("background.txt"); //background.txt를 unlink 한다
		close(fd);
		return 0;
	}

	pid = atoi(strtok(tmp, " ")); //공백 기준으로 tmp를 자르고 pid에 정수로 변환해서 대입
	close(fd);

	unlink("background.txt"); //background.txt를 unlink 한다
	return pid;
}

double compare_resultfile(char *file1, char *file2)
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY); //file1을 읽기 전용으로 열기
	fd2 = open(file2, O_RDONLY); //file2를 읽기 전용으로 열기

	while(1)
	{
		while((len1 = read(fd1, &c1, 1)) > 0){ //c1에 fd1에서 문자를 하나 읽어옴
			if(c1 == ' ')  //c1 공백이면 다음 반복 수행
				continue;
			else 
				break; //아니면 종료
		}
		while((len2 = read(fd2, &c2, 1)) > 0){ //c2에 fd2에서 문자를 하나 읽어옴
			if(c2 == ' ') //c2가 공백이면 다음 반복 수행
				continue;
			else 
				break; //아니면 종료
		}
		
		if(len1 == 0 && len2 == 0) //len1, len2 가 0이면 종료
			break;

		to_lower_case(&c1); //c1을 소문자로 변경
		to_lower_case(&c2); //c2를 소문자로 변경

		if(c1 != c2){ //c1, c2가 다른 값이면
			close(fd1);
			close(fd2);
			return false; //거짓 반환
		}
	}
	close(fd1);
	close(fd2);
	return true;
}

void redirection(char *command, int new, int old)
{
	int saved;

	saved = dup(old); //save에 old 파일 디스크립터 dup
	dup2(new, old); //new를 old에 dup

	system(command); //command 수행

	dup2(saved, old); //old에 saved를 dup
	close(saved); //saved를 닫는다
}

int get_file_type(char *filename)
{
	char *extension = strrchr(filename, '.'); //filename 의 확장자를 찾는다

	if(!strcmp(extension, ".txt")) //txt이면 TEXTFILE 리턴
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))//c이면 CFILE 리턴
		return CFILE;
	else
		return -1; //둘다 아니면 -1 리턴
}

void rmdirs(const char *path)
{
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[BUFLEN];
	
	if((dp = opendir(path)) == NULL) //path를 opendir 한다
		return;

	while((dirp = readdir(dp)) != NULL) //파일을 읽어온다
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) //파일 이름이 ., .. 이면 다음 반복 수행
			continue;
		sprintf(tmp,"%s/%s", path, dirp->d_name); //tmp에 path/파일이름 을 쓴다
		//printf("remove %s\n", tmp);

		if(lstat(tmp, &statbuf) == -1) //stat 구조체에 lstat을 통해 불러옴
			continue;

		if(S_ISDIR(statbuf.st_mode)) //파일이 디렉터리면 
			rmdirs(tmp); //rmdirs 수행
		else
			unlink(tmp); //tmp 파일 unlink
	}

	closedir(dp); //dp를 닫음
	rmdir(path); //path rmdir 실행
}

void to_lower_case(char *c) //대문자를 소문자로 변경
{
	if(*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage() //사용법을 출력하는 함수
{
	printf("Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
	printf("Option : \n");
    printf(" -m 	           modify question's score\n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file \n");
	printf(" -t <QNAMES>       compile QNAME.C with -lpthread option\n");
	printf(" -i <IDS>          print ID's wrong question\n");
	printf(" -h                print usage\n");
	
	
}
