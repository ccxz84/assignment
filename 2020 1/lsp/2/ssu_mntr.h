#define STUID "20162474"
#define MAXARG 5
#define BUFLEN 1024
#define FILELEN 64

#define TRASH "trash"
#define INFO "info"
#define FILES "files"
#define DIRECTORY "check"
#define MAXINFOFILE 64
#define MAXINFOSIZE 2048

struct file{
	char filename[FILELEN];
	struct file * next;
	struct file * dir;
	time_t mtime;
};

//ssu_mntr.c
int ssu_mntr(); //ssu_mntr 메인 프로그램 함수
void init(char (*command_token)[BUFLEN]); //commnad_token 변수 초기화 함수
int check_option(char * command,char (*command_token)[BUFLEN]); //사용자가 입력한 명령을 tokenize 하는 함
int runCommand(char (*command_token)[BUFLEN]); //사용자의 명령을 실행하는 함수
void delete_signal(int signo); //delete 시그널을 받아서 처리하는 함수
int help_commnad(); //help 명령을 실행하는 함수

//size.c
int size_command(char (*command_token)[BUFLEN]); //size 명령을 실행하는 함수
int search_directory(char * path, int ccount,int count); //파일, 디렉터리를 찾아 크기를 출력하는 함수

//delete.c
int delete_command(char (*command_token)[BUFLEN]); //제거 명령을 실행하는 함수
int trashDirectory_check(); //trash 디렉터리가 있는지 체크하는 함수
int make_info(char * path,char * filename); //info 파일을 만드는 함수
int delete_info(char * dPath); //info 파일을 오래된 순으로 제거하는 함수
char * get_info_num(char * buf); //중복된 파일이 몇개 있는지 체크하는 함수

//recover.c
int recover_command(char (*command_token)[BUFLEN]); //recover 명령을 실행하는 함수
int search_file(char * filename,int *  numlist); //사용자가 입력한 파일의 동일한 파일을 찾는 함수
int print_oldlist(); //파일을 오래된 순으로 정렬하여 출력하는 함수
time_t get_delete_time(char * filename); //파일의 제거 시간을 불러오는 함수

//tree.c
int tree_command(char (*command_token)[BUFLEN]); //tree 명령을 실행하는 함수
int print_tree(char * path,int (*filter)(const struct dirent *), int phase,int blank,int flag); //tree를 출력하는 함수

//lib.c
off_t get_directory_size(char const *path,int (*filter)(const struct dirent *)); //디렉터리의 사이즈를 반환하는 함수
int direc_filter(const struct dirent * dirp); //scandir에서의 리스트를 필터링하는 함수
struct tm *get_time_struct(char * day, char * time, struct tm * _time); //문자열 형태의 시간을 struct tm 구조체로 변경하는 함수
int uatoi(char s[]); //문자열을 숫자로 변환하는 함수
double pow(double x, double y); //x^y를 구해주는 함수
int delete_directory(char * path); //디렉터리를 제거하는 함수

//daemon.c
int ssu_daemon(); //daemon을 실행하는 함수
int daemon_init(struct dirent ** filelist,int count,struct file * head); //파일의 리스트를 초기화하는 함수
void creat_file(struct dirent  **filelist, int count,struct file * head); //생성된 파일을 체크하는 함수
void delete_file(struct dirent  **filelist, int count,struct file * head); //제거된 파일을 체크하는 함수
void modified_file(struct dirent ** filelist,int count,struct file * head); //수정된 파일을 체크하는 함수
void free_list(struct file * head); //리스트를 초기화하는 함수