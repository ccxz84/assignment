#define BUFLEN 1024

//rsync.c
int ssu_rsync(int argc, char *argv[]); //rsync 메인 함수
int checkotp(int argc, char *argv[]); //옵션을 판별하는 함수
void _sync(char * src, char * dst); //동기화의 전반적인 과정을 수행하는 함수
int direc_filter(const struct dirent * dirp); //scandir의 필터 함수
int sync_file(char * src, char * dst); //파일을 동기화하는 함수
void print_usage(); //사용법을 출력하는 함수
void make_randname(char *dir); //임의의 파일명을 생성하는 함수
void copy_directory(char const *src, char const *dst); //디렉터리를 복사하는 함수
void directory_compare(char * src, char * dst,char * path); //디렉터리를 비교하여 차이점을 찾는 함수
int delete_directory(char * path); //디렉터리를 제거하는 함수
void interrupt(int signo); //sigint에 대한 함수
void write_log(); //로그를 출력하는 함수
void _directory_compare(char * src, char * dst,char * path); //src에는 존재하지 않지만 dst에 존재하는 파일을 제거하는 함수
void print_log(char * path, char *filepath,int fd); //로그를 저장하는 함수
void runtoption(char * src, char * dst,char * path, char * mCommnad,int fd); //t옵션을 수행하는 함수