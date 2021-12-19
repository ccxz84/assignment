#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <dirent.h>
#include "ssu_mntr.h"

#define SECOND_TO_MICRO 1000000

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);

int main(int argc, char *argv[])
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);//시작시간 측정

	ssu_mntr();//ssu_mntr 실행
	sleep(2);//2초 대기
	gettimeofday(&end_t, NULL);//끝난시간 측정
	ssu_runtime(&begin_t, &end_t); //실행시간 출력


	exit(0);
}

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t)
{
	end_t->tv_sec -= begin_t->tv_sec;//끝난시간에서 시작시간을 뺌

	if(end_t->tv_usec < begin_t->tv_usec){//시작시간의 마이크로초가 끝난시간보다 크면
		end_t->tv_sec--;//끝난시간 값을 감소하고
		end_t->tv_usec += SECOND_TO_MICRO; //끝난시간의 마이크로초에 1000000을 더함
	}

	end_t->tv_usec -= begin_t->tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t->tv_sec, end_t->tv_usec); //실행시간 출력
}

