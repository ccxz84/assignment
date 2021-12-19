#ifndef MAIN_H_
#define MAIN_H_

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef STDOUT
	#define STDOUT 1
#endif
#ifndef STDERR
	#define STDERR 2
#endif
#ifndef TEXTFILE
	#define TEXTFILE 3
#endif
#ifndef CFILE
	#define CFILE 4
#endif
#ifndef OVER
	#define OVER 5
#endif
#ifndef OVER_SCORE
	#define OVER_SCORE 0.0
#endif
#ifndef WARNING
	#define WARNING -0.1
#endif
#ifndef ERROR
	#define ERROR 0.0
#endif

#define FILELEN 64
#define BUFLEN 1024
#define SNUM 100
#define QNUM 100
#define ARGNUM 5

struct ssu_scoreTable{
	char qname[FILELEN];
	double score;
};

void ssu_score(int argc, char *argv[]);
int check_option(int argc, char *argv[]);//인자를 확인하여 옵션을 체크하는 함수
void print_usage();//사용법을 출력해주는 함수

void score_students();//score.csv를 만들고 평균 점수를 띄워주는 함수
double score_student(int fd, char *id); //학생 개별 점수 계산
void write_first_row(int fd); //csv 첫번째 라인 sum 써주는 함수

char *get_answer(int fd, char *result); //블랭크 문제의 답을 리턴해주는 함수
int score_blank(char *id, char *filename); //블랭크 문제를 채점하기 위해 make token, make tree compare tree를 수행하는 함수
double score_program(char *id, char *filename); //프로그램 문제를 채점하기 위해 .c 코드를 컴파일 하고 실행하는 함수
double compile_program(char *id, char *filename); //.c 코드를 컴파일 하는 함수
double execute_program(char *id, char *filname); //컴파일 된 코드를 실행하는 함수
pid_t inBackground(char *name); //실행하는 프로그램의 pid를 가져오는 함수
double check_error_warning(char *filename); //error 파일을 탐색하여 warning 과 error 를 찾아 감점할 점수를 알려주는 함수
double compare_resultfile(char *file1, char *file2); //실행한 프로그램의 표준 출력을 비교하여 정답을 판별하는 함수

void do_iOption(char (*ids)[FILELEN]); //i 옵션 입력시 출력을 위한 함수
int read_gradeTable(FILE * fp, char (*qname)[FILELEN],char tmp[BUFLEN]);//점수 테이블을 불러오는 함수
int is_exist(char (*src)[FILELEN], char *target); //target 학생의 학번이 src배열에 존재하는지 확인하는 함수

int is_thread(char *qname); //qname 이 -t 옵션에 명시가 되어있는지 확인하는 함수
void redirection(char *command, int newfd, int oldfd); //oldfd의 출력을 newfd에 출력하는 명령을 실행하는 함수
int get_file_type(char *filename); //filename의 확장자를 확인하여 결과를 반환하는 함수
void rmdirs(const char *path); //path에 있는 모든 파일을 제거하는 함수
void to_lower_case(char *c); //대문자를 소문자로 바꿔주는 함수

void set_scoreTable(char *ansDir); //ansdir에서 score_table의 조작을 하는 함수(읽기, 쓰기)
void edit_question_grade(); //m 옵션이 있는 경우 문제의 배점을 수정하는 함수
void read_scoreTable(char *path); //path 에서 점수를 불러오는 함수 
void make_scoreTable(char *ansDir); //점수를 새로 생성하는 함수
void write_scoreTable(char *filename); //score_table 변수를 score_table.csv에 쓰는 함수
void set_idTable(char *stuDir); //학생답 폴더 경로를 받아 id_table에 학번을 입력하는 함수
int get_create_type(); //점수 테이블을 만들 시, 어떤식으로 생성할지 입력받는 함수

void sort_idTable(int size); //id_table을 정렬하는 함수
void sort_scoreTable(int size); //score_table를 정렬하는 함수
void get_qname_number(char *qname, int *num1, int *num2); //qname에서 문제번호를 찾아주는 함수

#endif
