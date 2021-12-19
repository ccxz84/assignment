#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include "rsync.h"

#define _DARWIN_C_SOURCE

int toption = 0;
int roption = 0;
int moption = 0;

char copydir[BUFLEN];
char dstdir[BUFLEN];

int log_fd = -1;

int ssu_rsync(int argc, char *argv[]){
	char src[BUFLEN],dst[BUFLEN], *stime, buf[BUFLEN];
	int index,i;
	time_t t;

	strcpy(copydir, "");
	strcpy(dstdir, "");

	time(&t);
	stime = ctime(&t);
	snprintf(buf,26,"[%s",stime);
	strcat(buf,"] ssu_rsync");
	

	signal(SIGINT, interrupt); //sigint 등록

	log_fd = fileno(tmpfile()); //log를 쓰기 위한 임시파일을 열기
	
	
	if(argc < 3){ //인자 객수 확인
		print_usage();
		exit(1);
	}

	for(i = 1; i< argc;i++){ //인자를 buf에 붙임
		strcat(buf, " ");
		strcat(buf, argv[i]);
	}
	strcat(buf, "\n");
	write(log_fd, buf, strlen(buf)); //buf를 로그에 씀

	index = checkotp(argc,argv); //옵션 체크함수 실행
	if(index +1 > argc){ //인자 수가 맞지 않은경우 사용방법 실행
		print_usage();
		exit(1);
	}
	strcpy(src,argv[index]); //src, dst에 인자를 넣음
	strcpy(dst,argv[index+1]);
	_sync(src,dst);//_sync 함수 실행
	write_log();
	close(log_fd);
}	

int checkotp(int argc, char *argv[]){
	char c;
	int ind = 1;
	while((c = getopt(argc, argv, "e:trm")) != -1) {
		switch(c){
			case 't': //t 옵션 체크
				ind = optind;
				toption = 1;
				break;
			case 'r': //r 옵션 체크
				ind = optind;
				roption = 1;
				break;
			case 'm': //m 옵션 체크
				ind = optind;
				moption = 1;
				break;
			case '?': //조건에 해당하지 않을시 사용법 출력
				print_usage();
				exit(1);
				break;
		}
	}
	return ind;
}

void print_usage(){ //사용법 출력을 위한 함수
	fprintf(stderr, "usage : ./ssu_rsync [option] [src] [dst]\n");
}

void _sync(char * src, char * dst){
	int scount,dcount,i,j,flag = 0, pathfd,tmpfd,size;
	struct dirent ** snamelist, **dnamelist, *sdirp, *ddirp;
	char *realsrc, *realdst, sfilename[BUFLEN],*ptr,*str, buf[BUFLEN], *cwd, tmpdir[BUFLEN], mCommnad[BUFLEN];
	struct stat Sstatbuf,Dstatbuf,statbuf;
	FILE * fp;

	realsrc = realpath(src,NULL); //src 경로의 절대경로 파악
	realdst = realpath(dst,NULL); //dst 경로의 절대경로 파악

	if(lstat(realsrc,&Sstatbuf) < 0){ //src 경로의 스탯 구조체를 가져옴
		fprintf(stderr, "src file not found\n");
		exit(1);
	}

	if(lstat(realdst, &Dstatbuf) < 0){ //dst 경로의 스탯 구조체를 가져옴
		fprintf(stderr, "dst not fount\n");
		exit(1);
	}

	if(!S_ISDIR(Dstatbuf.st_mode)){ //dst 경로가 디렉터리인지 확인
		fprintf(stderr, "dst is not directory\n");
		exit(1);
	}

	cwd = getcwd(NULL,0);//작업디렉터리 파악
	memset(tmpdir, 0 , BUFLEN);
	make_randname(tmpdir); //랜덤한 디렉터리 명 생성
	strcpy(buf, cwd);
	strcat(buf, "/");
	strcat(buf, tmpdir);
	strcpy(copydir,buf);//copydir에 작업디렉터리/랜덤 디렉터리명 복사
	strcpy(dstdir, realdst); //realdst 경로를 dstdir 에 복사
	copy_directory(realdst,buf); //dst 경로의 내용을 copydir에 백업

	dcount = scandir(realdst,&dnamelist,direc_filter,alphasort);

	if(!S_ISDIR(Sstatbuf.st_mode)){ //src 경로의 파일이 디렉터리가 아닌경우
		ptr = src;
		str = ptr;
		while(ptr < src + strlen(src)){
			if(*ptr == '/'){

				str = ptr+1;
				
			}
			ptr++;
		}
		
		strcpy(sfilename,str); //sfilename에 src 파일의 파일명을 복사
		for(j=0;j<dcount;j++){ //dst 디렉터리에 대해 반복 수행

			ddirp = dnamelist[j];
			strcpy(buf, realdst);
			if(!strcmp(ddirp->d_name, sfilename)){//dst 디렉터리의 파일명과 sfilename이 같으면
				strcat(buf, "/");
				strcat(buf, ddirp->d_name);
				if(lstat(buf, &statbuf) < 0){
					fprintf(stderr, "lstat error for %s\n",buf);
					continue;
				}

				if(statbuf.st_size == Sstatbuf.st_size && statbuf.st_mtime == Sstatbuf.st_mtime)
					break;

				sync_file(realsrc, realdst); //파일을 동기화함
				flag = 1;
			}
		}
		if(flag == 0){ //dst에 파일이 존재하지 않으면
			strcpy(buf, realdst);
			strcat(buf, "/");
			strcat(buf, sfilename);
			sync_file(realsrc,buf); //파일을 생성
		}
	}
	if(toption == 1){ //t옵션이 켜져있는 경우
		memset(buf, 0, sizeof(buf));
		tmpfd = fileno(tmpfile()); //임시 로그파일 생성
		make_randname(buf); //임시 파일명 생성
		sprintf(mCommnad, "tar -cvf %s.tar ",buf); 
		runtoption(realsrc,realdst,"",mCommnad,tmpfd); //runtoption 수행
		chdir(src);//작업디렉터리 변경
		system(mCommnad); //tar -cvf 임시파일명.tar 동기화 대상파일 명령 수행
		sprintf(mCommnad, "%s.tar",buf);
		lstat(mCommnad, &statbuf);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "		totalsize %ldbytes\n",statbuf.st_size); //tar의 크기를 로그에 쓰기
		write(log_fd, buf, strlen(buf));
		lseek(tmpfd, 0, SEEK_SET);
		while((size = read(tmpfd, buf, BUFLEN)) > 0){ //tmpfd를 통해 쓰여진 임시 로그를 로그파일로 옮김
			write(log_fd, buf, size);
		}
		sprintf(buf, "tar -xvf %s -C %s",mCommnad, realdst); //tar 해체를 dst에 수행
		system(buf);
		remove(mCommnad); //tar 파일 삭제
		chdir(cwd); //작업디렉터리 변경
	}
	else{
		directory_compare(realsrc,realdst,""); //directory_compare 수행
		if(moption == 1)//moption이 켜진경우
			_directory_compare(realsrc,realdst,""); //_directory_compare 수행
	}
	
	delete_directory(copydir); //백업 디렉터리 삭제

}

void write_log(){
	int fd,size;
	char buf[BUFLEN];

	if(access("ssu_rsync_log", F_OK) < 0){ //로그파일이 없는 경우 생성
		fd = open("ssu_rsync_log", O_WRONLY|O_CREAT, 0644);
	}
	else{
		fd = open("ssu_rsync_log", O_WRONLY|O_APPEND|O_CREAT); //로그파일 열기
	}

	if(fd < 0){ //로그파일 에러 확인
		fprintf(stderr, "log open error\n");
		exit(1);
	}
	lseek(log_fd, 0, SEEK_SET); //임시 로그파일의 첫 지점으로 offset 이동
	while((size = read(log_fd, buf, BUFLEN)) > 0){ //임시로그파일에서 로그파일로 내용 옮김
		write(fd, buf, size);
	}
	close(fd);
}

void directory_compare(char * src, char * dst,char * path){
	int scount, dcount,flag, sdir, i, j,fileflag;
	struct dirent *sdirp , *ddirp, **snamelist, **dnamelist;
	char sfile[BUFLEN],dfile[BUFLEN],filepath[BUFLEN],buf[BUFLEN];
	struct stat sstatbuf, dstatbuf;
	struct utimbuf times;

	scount = scandir(src,&snamelist,direc_filter,alphasort);
	dcount = scandir(dst,&dnamelist,direc_filter,alphasort);


	for(i = 0;i<scount;i++){
		sdir = 0;
		flag = 0;
		fileflag = 0;
		sdirp = snamelist[i];
		strcpy(sfile, src);
		strcat(sfile, "/");
		strcat(sfile, sdirp->d_name); //src경로/파일명 을 가지는 문자열 생성
		lstat(sfile, &sstatbuf); //파일에 대한 stat 구조체를 가져옴
		strcpy(filepath,path);
		strcat(filepath,sdirp->d_name); //dst 경로에 대한 상대경로 생성
		if(S_ISDIR(sstatbuf.st_mode)) //동기화 파일이 디렉터리인지 판별
			sdir = 1;
		for(j = 0;j<dcount;j++){
			ddirp = dnamelist[j];
			strcpy(dfile, dst);
			strcat(dfile, "/");
			strcat(dfile, ddirp->d_name); //dst경로/파일명 을 가지는 문자열 생성
			lstat(dfile, &dstatbuf);
			strcpy(filepath,path);
			strcat(filepath,ddirp->d_name); //dst 경로에 대한 상대경로 생성
			if(!strcmp(ddirp->d_name, sdirp->d_name)){ //파일명이 같은경우
				flag = 1;
				if(S_ISDIR(dstatbuf.st_mode)){ //만약 디렉터리라면
					if(sdir == 1 && roption == 1){//r옵션이 켜져있고 src 의 해당 파일도 디렉터리라면
						strcat(filepath,"/");
						directory_compare(sfile, dfile,filepath);//directory_compare 재귀적으로 실행
						break;
					}
					else if(roption == 1){ //r옵션이 켜져있고 src의 파일이 디렉터리가 아닌경우
						delete_directory(dfile); //dst 경로의 디렉터리를 삭제
						sprintf(buf, "		%s %ldbytes\n",filepath,sstatbuf.st_size);
						write(log_fd, buf, strlen(buf)); //로그 기록
						sync_file(sfile, dfile); //파일 동기화
						break;
					}
				}
				else{ //dst 파일이 디렉터리가 아닌경우
					if(sdir == 1 && roption == 1){//src의 파일이 디렉터리이고 r 옵션이 켜져 있는 경우
						unlink(dfile); //dst 파일 삭제
						fileflag = sstatbuf.st_mode;
						fileflag &= 0777; //파일권한을 가져옴
						times.actime = sstatbuf.st_atime;
						times.modtime = sstatbuf.st_mtime;//시간을 설정함
						mkdir(dfile,fileflag); //디렉터리 생성
						strcat(filepath,"/");
						directory_compare(sfile, dfile,filepath); //directory_compare 재귀적으로 수행
						utime(dfile, &times); //수정시간 변경
						break;
						//dfile 파일 삭제후 sfile dfile 로 복사
					}
					else if(sdir == 1) //r옵션이 꺼져 있으면 건너뜀
						continue;
					else if(dstatbuf.st_size == sstatbuf.st_size && dstatbuf.st_mtime == sstatbuf.st_mtime){ //둘다 디렉터리가 아닌경우 파일이 다름을 판별
						break;
					}
					//파일이 다르면 파일 동기화 수행
					sprintf(buf, "		%s %ldbytes\n",filepath,sstatbuf.st_size);
					write(log_fd, buf, strlen(buf));
					sync_file(sfile, dfile);
				}
			}
		}
		//dst에 파일이 존재하지 않는 경우
		if(sdir == 1 && flag == 0 && roption == 1){ //src의 파일이 디렉터리인 경우
			strcpy(dfile, dst);
			strcat(dfile, "/");
			strcat(dfile, sdirp->d_name);
			fileflag = sstatbuf.st_mode;
			fileflag &= 0777; //파일 권한 가져옴
			mkdir(dfile,fileflag); //dst 디렉터리 생성
			strcpy(filepath,path);
			strcat(filepath,sdirp->d_name);
			strcat(filepath,"/");
			times.actime = sstatbuf.st_atime;
			times.modtime = sstatbuf.st_mtime;
			
			directory_compare(sfile, dfile,filepath);//directory_compare 재귀적으로 실행
			utime(dfile, &times);
			//sfile 디렉터리 을 dst/sdirp->d_name 에 복사
		}
		else if(sdir == 0 && flag == 0){//src의 파일이 파일인 경우
			strcpy(dfile, dst);
			strcat(dfile, "/");
			strcat(dfile, sdirp->d_name);
			strcpy(filepath,path);
			strcat(filepath,sdirp->d_name);
			sprintf(buf, "		%s %ldbytes\n",filepath,sstatbuf.st_size);
			write(log_fd, buf, strlen(buf));//로그 기록
			sync_file(sfile, dfile);//파일 동기화
			//sfile 파일 을 dst/sdirp->d_name 에 복사
		}
	}
}

void _directory_compare(char * src, char * dst,char * path){
	int scount, dcount,flag, sdir, i, j,fileflag;
	struct dirent *sdirp , *ddirp, **snamelist, **dnamelist;
	char sfile[BUFLEN],dfile[BUFLEN],filepath[BUFLEN],buf[BUFLEN];
	struct stat sstatbuf, dstatbuf;
	struct utimbuf times;

	scount = scandir(src,&snamelist,direc_filter,alphasort);
	dcount = scandir(dst,&dnamelist,direc_filter,alphasort);

	for(j = 0;j < dcount;j++){ //dst 디렉터리에 대해 반복 수행
		flag = 0;
		ddirp = dnamelist[j];
		strcpy(dfile, dst);
		strcat(dfile, "/");
		strcat(dfile, ddirp->d_name);//dst 경로의 절대경로 생성
		strcpy(filepath,path);
		strcat(filepath,ddirp->d_name); //dst 경로에 대해 상대경로 생성
		for(i = 0;i < scount;i++){
			sdirp = snamelist[i];
			strcpy(sfile, src);
			strcat(sfile, "/");
			strcat(sfile, sdirp->d_name);//src 경로의 절대경로 생성
			lstat(sfile, &sstatbuf);
			if(!strcmp(ddirp->d_name, sdirp->d_name)){ //같은 파일이 존재하는지 확인
				flag = 1;
				if(S_ISDIR(sstatbuf.st_mode)){//파일이 디렉터리인경우
					strcat(filepath,"/");
					_directory_compare(sfile,dfile,filepath);//_directory_compare 재귀적으로 수행
				}
				break;
			}
		}
		if(flag != 1){ //파일이 src 경로에 존재하지 않으면
			lstat(dfile, &dstatbuf);
			if(S_ISDIR(dstatbuf.st_mode) && roption == 1){ //r옵션이 켜져있고 디렉터리이면
				sprintf(buf, "		%s delete\n",filepath);
				write(log_fd, buf, strlen(buf));//로그 기록
				delete_directory(dfile);//디렉터리 삭제
			}
			else if(!S_ISDIR(dstatbuf.st_mode)){//아닌 경우
				sprintf(buf, "		%s delete\n",filepath);
				write(log_fd, buf, strlen(buf));//로그 기록
				unlink(dfile); //파일삭제
			}
		}
	}
}

void runtoption(char * src, char * dst,char * path, char * mCommnad, int fd){
	int scount, dcount,flag, sdir, i, j,fileflag;
	struct dirent *sdirp , *ddirp, **snamelist, **dnamelist;
	char sfile[BUFLEN],dfile[BUFLEN],filepath[BUFLEN],buf[BUFLEN];
	struct stat sstatbuf, dstatbuf;
	struct utimbuf times;

	scount = scandir(src,&snamelist,direc_filter,alphasort);
	dcount = scandir(dst,&dnamelist,direc_filter,alphasort);


	for(i = 0;i<scount;i++){
		sdir = 0;
		flag = 0;
		fileflag = 0;
		sdirp = snamelist[i];
		strcpy(sfile, src); 
		strcat(sfile, "/");
		strcat(sfile, sdirp->d_name);//src경로/파일명 을 가지는 문자열 생성
		lstat(sfile, &sstatbuf); //파일에 대한 stat 구조체를 가져옴
		strcpy(filepath,path);
		strcat(filepath,sdirp->d_name);//dst 경로에 대한 상대경로 생성
		if(S_ISDIR(sstatbuf.st_mode))//동기화 파일이 디렉터리인지 판별
			sdir = 1;
		for(j = 0;j<dcount;j++){
			ddirp = dnamelist[j];
			strcpy(dfile, dst);
			strcat(dfile, "/");
			strcat(dfile, ddirp->d_name);//dst경로/파일명 을 가지는 문자열 생성
			lstat(dfile, &dstatbuf);
			if(!strcmp(ddirp->d_name, sdirp->d_name)){ //파일명이 같다면
				strcpy(filepath,path);
				strcat(filepath,ddirp->d_name);//dst 경로에 대한 상대경로 생성
				flag = 1;
				if(sdir == 1 && roption == 1 && S_ISDIR(dstatbuf.st_mode)){ //파일이 디렉터리이고 r옵션이 켜져 있으며 src 파일이 디렉터리인경우
					strcat(filepath,"/");
					runtoption(sfile,dfile,filepath,mCommnad,fd);//runtoption 재귀적을 수행
					break;
				}
				if(dstatbuf.st_size == sstatbuf.st_size && dstatbuf.st_mtime == sstatbuf.st_mtime && sstatbuf.st_mode == dstatbuf.st_mode){//다른 파일인지 비교
					break;
				}
				else if(sdir == 1){ //src 파일이 디렉터리이이며, dst에 디렉터리가 아닌 상태로 존재하는경우
					strcat(filepath,"/");
					print_log(dfile, filepath,fd); //로그 출력
				}
				else{ //src 파일이 디렉터리가 아니며 dst에 존재하는 경우
					sprintf(buf, "		%s\n",filepath);
					write(fd, buf, strlen(buf));//로그 출력
				}
				strcat(mCommnad, filepath);
				strcat(mCommnad, " "); //tar 명령에 인자로 추가
			}
		}
		if(sdir == 1 && flag == 0 && roption == 1){//src 파일이 디렉터리이며 r 옵션이 켜져있고 dst에 존재하지 않는경우
			strcat(filepath,"/");
			print_log(sfile, filepath,fd); //로그 출력
			strcat(mCommnad, filepath);
			strcat(mCommnad, " ");//tar 명령에 인자로 추가
		}
		else if(sdir == 0 && flag == 0){
			sprintf(buf, "		%s\n",filepath);
			write(fd, buf, strlen(buf));//로그 출력
			strcat(mCommnad, filepath);
			strcat(mCommnad, " ");//tar 명령에 인자로 추가
		}
	}
}

void print_log(char * path, char *filepath, int fd){
	struct dirent ** namelist;
	char srcbuf[BUFLEN],fbuf[BUFLEN],buf[BUFLEN];
	int count, i;
	struct stat statbuf;

	count = scandir(path,&namelist,direc_filter,alphasort);	//scandir 수행

	for(i=0;i<count;i++){ //filelist에 대해 반복
		strcpy(srcbuf,path);
		strcat(srcbuf, "/");
		strcat(srcbuf, namelist[i]->d_name);
		strcpy(fbuf,filepath);
		strcat(fbuf,namelist[i]->d_name);
		if(lstat(srcbuf,&statbuf) < 0){ //파일의 stat 구조체를 가져옴
			fprintf(stderr,"%s stat error\n",namelist[i]->d_name);
			continue;
		}

		if(S_ISDIR(statbuf.st_mode)){ //파일이 디렉터리라면
			strcat(fbuf, "/");
			print_log(srcbuf, fbuf,fd);//print_log 재귀적으로 수행
		}
		sprintf(buf, "		%s\n",fbuf);
		write(fd, buf, strlen(buf));//로그 출력
		free(namelist[i]);
	}
	free(namelist);
}

int direc_filter(const struct dirent * dirp){
	if(!(strcmp(dirp->d_name,"."))||!(strcmp(dirp->d_name,".."))){ //디렉터리가 ., .. 이면 제거
		return 0;
	}
}

int sync_file(char * src, char * dst){
	int sfd, dfd, size;
	struct stat statbuf;
	char buf[BUFLEN];
	struct utimbuf times;
	int flag;

	if(lstat(src, &statbuf) < 0){ //src 파일의 스탯 구조체를 가져옴
		fprintf(stderr, "lstat error for %s\n",src);
		return 0;
	}

	if((sfd = open(src, O_RDONLY)) < 0){ //src 파일 열기
		fprintf(stderr, "src open error for %s\n",src);
		return 0;
	}

	flag = statbuf.st_mode;
	flag &= 0777; //파일 권한을 가져옴

	if((dfd = open(dst, O_CREAT|O_WRONLY|O_TRUNC, 0644)) < 0){	//목적 파일을 쓰기 전용으로 열기
		fprintf(stderr, "src : %s dst open error for %s\n",src,dst);
		return 0;
	}

	while((size = read(sfd, buf, BUFLEN)) > 0) //파일의 내용 복사
		write(dfd, buf, size);

	times.actime = statbuf.st_atime;
	times.modtime = statbuf.st_mtime; //시간 정보를 가져옴

	close(sfd);
	close(dfd);

	chmod(dst,flag);//권한 변경
	utime(dst, &times); //시간 변경
	
}

void make_randname(char * dir){
	
	int i;
	char t;

	srand(time(NULL));

	for(i = 0;i < 5;i++){
		t = rand()%128;
		while(!(isdigit(t)) && !(isalpha(t))){ //랜덤한 문자중 숫자, 문자가 아닌경우 반복
			t = rand()%128;
		}
		dir[i] = t;
		
	}
}

void copy_directory(char const *src, char const *dst){
	char buf[BUFLEN],srcbuf[BUFLEN];
	struct dirent ** namelist, *dirp;
	int i,count,flag;
	struct stat statbuf;
	struct utimbuf times;

	count = scandir(src,&namelist,direc_filter,alphasort); //파일경로에 대해 scandir 수행

	if(lstat(src,&statbuf) < 0){
		fprintf(stderr, "%s stat error\n",src);
		return;
	}
	mkdir(dst,statbuf.st_mode & 0777); //디렉터리 생성
	lstat(dst, &statbuf);
	times.actime = statbuf.st_atime;
	times.modtime = statbuf.st_mtime;
	for(i=0;i<count;i++){
		flag = 0;
		dirp = namelist[i];
		strcpy(buf, dst);
		strcat(buf, "/");
		strcat(buf, dirp->d_name);
		strcpy(srcbuf, src);
		strcat(srcbuf, "/");
		strcat(srcbuf, dirp->d_name);
		if(lstat(srcbuf,&statbuf)){ //파일의 stat구조체를 가져옴
			fprintf(stderr,"stat error\n");
			break;
		}
		if(S_ISDIR(statbuf.st_mode)){ //파일이 디렉터리인지 체크
			copy_directory(srcbuf,buf); //디렉터리 체크 함수를 실행하고 결과를 size에 더함
			continue;
		} 
		sync_file(srcbuf,buf);
	}
	utime(dst, &times);
}

int delete_directory(char * path){
	struct dirent ** namelist;
	char srcbuf[BUFLEN];
	int count, i;
	struct stat statbuf;

	count = scandir(path,&namelist,direc_filter,alphasort);	//scandir 수행

	for(i=0;i<count;i++){ //filelist에 대해 반복
		strcpy(srcbuf,path);
		strcat(srcbuf, "/");
		strcat(srcbuf, namelist[i]->d_name);
		if(lstat(srcbuf,&statbuf) < 0){ //파일의 stat 구조체를 가져옴
			fprintf(stderr,"%s stat error\n",namelist[i]->d_name);
			continue;
		}

		if(S_ISDIR(statbuf.st_mode)){ //파일이 디렉터리라면
			delete_directory(srcbuf); //디렉터리 제거함수 수행
		}
		remove(srcbuf); //파일 제거
		free(namelist[i]);
	}
	free(namelist);
	remove(path);
}

void interrupt(int signo){ //sigint를 받은경우
	delete_directory(dstdir); //dstdir 삭제
	copy_directory(copydir,dstdir); //백업 디렉터리를 복사
	delete_directory(copydir); //백업디렉터리 삭제
	exit(0);
}