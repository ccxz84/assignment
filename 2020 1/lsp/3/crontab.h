#define STUID "20162474"
#define MAXARG 32
#define BUFLEN 1024
#define FILELEN 64
#define ADDCOMMAND 5
#define MAXADDCOMMANDNUM 60

struct data {
	long long minute;
	long long hour;
	long long day;
	long long month;
	long long DoW;
	char command[BUFLEN];
	char origin[BUFLEN];
};

struct list{
	struct data _data;
	struct list * next;
};

int ssu_crontab(void); //ssu_crontab 프로그램 실행을 위한 함수
void init(char (*command_token)[BUFLEN]); //사용자 명령을 받는 변수를 초기화하는 함수
int check_option(char * command,char (*command_token)[BUFLEN]); //사용자의 명령을 체크하여 공백단위로 쪼개는 함수
int runCommand(char (*command_token)[BUFLEN]); //사용자의 명령에 따라 명령을 수행하는 함수
int exit_command(char (*command_token)[BUFLEN]); //exit 명령을 수행하는 함수

//add.c
int add_command(char (*command_token)[BUFLEN]); //add명령을 수행하는 함수
int valid_cycle(char (*command)[BUFLEN]); //주기가 유효한지 검사하는 함수
int add_file(FILE * fp,char (*command)[BUFLEN], char (*arg)[BUFLEN]); //유효한 명령을 파일에 기록하는 함수

//lib.c
int uatoi(char s[]); //문자열을 정수로 바꾸는 함수
long long ipow(int x, int y); //x^y 를 구하는 함수
int write_log(FILE * fp,char *cmd,char (*command)[BUFLEN], char (*arg)[BUFLEN]); //로그를 작성하는 함수
int print_cmd_list(FILE * fp); //명령을 터미널에 출력해주는 함수
int Addparse_command(char (*command)[BUFLEN], struct data * _data); //등록되어 있는 명령의 주기를 parsing 하는 함수
int addcommand_addflag(struct data * _data,int command,long long a1, int a2, char flag); //parsing 한 데이터를 struct data 변수에 추가하는 함수
long long getlistflag(int command,int a1, int a2); //a1-a2 형식과 같은 주기를 parsing 하는 함수

//daemon.c
void free_list(struct list * head); //linked-list를 초기화하는 함수
int daemon_init(FILE * fp,struct list * head); //daemon 프로그램의 linked-list 초기화 및 초기 세팅을 위한 함수
int time_check(struct list * cur, long long list[]); //각 명령의 시간 체크를 위한 함수
void run_command(FILE * fp, struct list * head); //시간이 일치한 명령을 실행하는 함수

//remove.c
int remove_command(char (*command_token)[BUFLEN]); //remove 명령을 수행하는 함수