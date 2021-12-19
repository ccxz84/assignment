#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "blank.h"

char datatype[DATATYPE_SIZE][MINLEN] = {"int", "char", "double", "float", "long"
			, "short", "ushort", "FILE", "DIR","pid"
			,"key_t", "ssize_t", "mode_t", "ino_t", "dev_t"
			, "nlink_t", "uid_t", "gid_t", "time_t", "blksize_t"
			, "blkcnt_t", "pid_t", "pthread_mutex_t", "pthread_cond_t", "pthread_t"
			, "void", "size_t", "unsigned", "sigset_t", "sigjmp_buf"
			, "rlim_t", "jmp_buf", "sig_atomic_t", "clock_t", "struct"}; //데이터 타입 정의


operator_precedence operators[OPERATOR_CNT] = {
	{"(", 0}, {")", 0}
	,{"->", 1}
	,{"*", 4}	,{"/", 3}	,{"%", 2}
	,{"+", 6}	,{"-", 5}
	,{"<", 7}	,{"<=", 7}	,{">", 7}	,{">=", 7}
	,{"==", 8}	,{"!=", 8}
	,{"&", 9}
	,{"^", 10}
	,{"|", 11}
	,{"&&", 12}
	,{"||", 13}
	,{"=", 14}	,{"+=", 14}	,{"-=", 14}	,{"&=", 14}	,{"|=", 14}
}; //연산자 우선순위 지정

void compare_tree(node *root1,  node *root2, int *result) //학생답, 정답 트리를 인자로 받음
{
	node *tmp;
	int cnt1, cnt2;

	if(root1 == NULL || root2 == NULL){ //둘중 하나가 널이면 에러
		*result = false;
		return;
	}

	if(!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")){ //루트1의 값이 부등호라면?
		if(strcmp(root1->name, root2->name) != 0){//루트1과 루트2의 다르다면?
			//루트2의 부등호를 반대로 바꿈
			if(!strncmp(root2->name, "<", 1))
				strncpy(root2->name, ">", 1);

			else if(!strncmp(root2->name, ">", 1))
				strncpy(root2->name, "<", 1);

			else if(!strncmp(root2->name, "<=", 2))
				strncpy(root2->name, ">=", 2);

			else if(!strncmp(root2->name, ">=", 2))
				strncpy(root2->name, "<=", 2);

			root2 = change_sibling(root2); //자식의 next와 자식의 위치 변경
		}
	}

	if(strcmp(root1->name, root2->name) != 0){ //root1의 name과 root의 name 이 같지 않다면
		*result = false; //거짓 리턴
		return;
	}

	if((root1->child_head != NULL && root2->child_head == NULL) //root1의 자식이 널이 아니면서 roo2의 자식이 널이거나 root1의 자식이 널이면서 roo2의 자식이 널이아니라면
		|| (root1->child_head == NULL && root2->child_head != NULL)){
		*result = false; //거짓 리턴
		return;
	}

	else if(root1->child_head != NULL){ //root1의 자식이 널이 아니리라면
		if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){ //root1의 자식의 next 갯수랑 root2의 자식의 next 갯수가 다르다면 거짓 반환
			*result = false;
			return;
		}

		if(!strcmp(root1->name, "==") || !strcmp(root1->name, "!=")) //root1의 값이 == 이거나 != 이라면
		{
			compare_tree(root1->child_head, root2->child_head, result); //roo1t1, root2 의 자식에 대한 compare_tree 수행

			if(*result == false) //결과가 거짓이라면
			{
				*result = true; //결과를 참으로 변경
				root2 = change_sibling(root2); //root2의 자식의 next와 자식의 위치 변환
				compare_tree(root1->child_head, root2->child_head, result); ///roo1t1, root2 의 자식에 대한 compare_tree 수행
			}
		}
		else if(!strcmp(root1->name, "+") || !strcmp(root1->name, "*") //root1이 +,*,|,&,|,||,&& 일때
				|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
				|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&"))
		{
			if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){ //root1의 자식의 next 갯수랑 root2의 자식의 next 갯수가 다르다면 result에 false 대입
				*result = false;
				return;
			}

			tmp = root2->child_head; //tmp에 root2의 자식을 대입

			while(tmp->prev != NULL) //tmp의 이전 노드가 null이 아니면 이전노드로 계속 이동
				tmp = tmp->prev;

			while(tmp != NULL) //tmp가 널이아니라면
			{
				compare_tree(root1->child_head, tmp, result); //root1의 자식과 tmp에 대한 compare_tree 수행

				if(*result == true) //결과가 참이면 중단
					break;
				else{
					if(tmp->next != NULL) //tmp의 next가 널이 아니면 결과는 참
						*result = true;
					tmp = tmp->next; //tmp를 다음노드로 진행
				}
			}
		}
		else{
			compare_tree(root1->child_head, root2->child_head, result); //root1의 자식과 root2의 자식에 대해 compare_tree 수행
		}
	}


	if(root1->next != NULL){ //root1의 next가 널이 아닐때

		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)){ //root1과 root2의 자식 갯수가 다르다면
			*result = false;
			return;
		}

		if(*result == true) //결과가 참이라면
		{
			tmp = get_operator(root1); //root1의 연산자 노드를 tmp에 대입

			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*")
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&")) //tmp의 값이 +,*,|,&,||,&& 라면
			{
				tmp = root2; //tmp에 root2를 대입

				while(tmp->prev != NULL) //tmp의 prev가 널이 아닐때까지 prev로 이동
					tmp = tmp->prev;

				while(tmp != NULL) //tmp가 널이 아니라면
				{
					compare_tree(root1->next, tmp, result); //root1의 next 와 tmp 의 compare_tree 수행

					if(*result == true) //결과가 참이면 중단
						break;
					else{
						if(tmp->next != NULL) //tmp의 next가 널이 아니면 결과를 참으로 변경
							*result = true;
						tmp = tmp->next; //tmp를 tmp->next 로 변경
					}
				}
			}

			else
				compare_tree(root1->next, root2->next, result); //root1, root2의 next에 대해 compare_tree 수행
		}
	}
}

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN])
{
	char *start, *end;
	char tmp[BUFLEN];
	char str2[BUFLEN];
	char *op = "(),;><=!|&^/+-*\"";
	int row = 0;
	int i;
 	int isPointer;
	int lcount, rcount;
	int p_str;

	clear_tokens(tokens); //tokens 초기화

	start = str;//start를 학생답으로 초기화

	if(is_typeStatement(str) == 0) //데이터 타입, gcc에 대한 문법이 맞는지 확인
		return false;

	while(1)
	{
		if((end = strpbrk(start, op)) == NULL) //start 에서 op에 해당하는 글자가 있으면 맨 첫번째 포인터를 end에 대입
			break;//널이면 브레이크

		if(start == end){ //start랑 end랑 같다면

			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)){//start앞에 '--', '++' 인지 확인
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4))//맨앞에 '----', '++++' 이면 false
					return false;

				if(is_character(*ltrim(start + 2))){//start에서 '--', '++' 두개를 뺀, 뒤의 문자가 숫자, 영어인지 확인
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]))//일단 무엇을 의미하는지 파악하기 힘듬
						return false;

					end = strpbrk(start + 2, op);//start에서 op를 가지는 문자열의 포인터를 end에 대입
					if(end == NULL)
						end = &str[strlen(str)];//학생답의 맨 마지막 을 end에 대입
					while(start < end) { //start가 end 보다 작으면(포인터)
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))//start -1의 문자가 공백이 token[row] 끝의 전문자가 문자 이면
							return false; //false 반환
						else if(*start != ' ') //아니면 start 처음을 공백으로 변경
							strncat(tokens[row], start, 1); //tokens의 row 인덱스의 값에 공백을 연결
						start++; //start 포인터의 값 증가
					}
				}

				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ //token의 인덱스가 가 0보다 크고, token의 row-1 인덱스의 마지막 문자가 숫자 또는 영어일때,
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL)//token의 row-1 인덱스에 ++또는 --가 포함되어 있다면 false 리턴
						return false;

					memset(tmp, 0, sizeof(tmp)); //tmp의 메모리를 0으로 초기화
					strncpy(tmp, start, 2); //start의 2개문자를 tmp에 복사
					strcat(tokens[row - 1], tmp); //tokens의 row-1 인덱스의 문자열에 tmp를 붙임
					start += 2; //start 포인터 2 증가
					row--; //row 값 감소
				}
				else{
					memset(tmp, 0, sizeof(tmp)); //tmp의 메모리를 0으로 초기
					strncpy(tmp, start, 2);
					strcat(tokens[row], tmp);
					start += 2;//start 포인터 2 증가
				}
			}

			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2)
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2)
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2)
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2)
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){ //start 앞의 두글 에 ==,!=, <=, >=, ||, &&, &=, ^=, !=, |=, +=, -=, *=, /= 가 있다면

				strncpy(tokens[row], start, 2); //tokens 의 row 인덱스에 위의 문자를 복사
				start += 2; //start 포인터 2 증가
			}
			else if(!strncmp(start, "->", 2)) //start의 2글자가 -> 라면
			{
				end = strpbrk(start + 2, op); //end에 연산자랑 일치되는 값을 -> 에서 찾아 넣는다

				if(end == NULL) //op의 값을 찾지 못하면
					end = &str[strlen(str)]; //end에 문자열의 마지막 값을 넣음

				while(start < end){ //start에서 end까지
					if(*start != ' ')//start가 공백이 아니면
						strncat(tokens[row - 1], start, 1); //tokens의 row-1에 start의 문자를 붙인다.
					start++;
				}
				row--; //row 값은 감소
			}
			else if(*end == '&') //찾은 연산자가 &일때
			{

				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){ //row가 0이고 token의 row-1 인덱스에 op의 연산자가 있다면
					end = strpbrk(start + 1, op); //end에 start+1 포인터에서 op를 찾은 포인터를 대입
					if(end == NULL)
						end = &str[strlen(str)]; //찾지 못하면 str의 맨 끝문자의 주소를 대입

					strncat(tokens[row], start, 1); //tokens의 row에 start 문자를 하나 붙이고 start 증가
					start++;

					while(start < end){ //start에서 end까지 반복
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&') //start-1 이 공백이고 tokens의 row 인덱스의 맨 끝 글자가 &가 아니라면
							return false; //거짓 반환
						else if(*start != ' ') //start 의 문자가 공백이면
							strncat(tokens[row], start, 1); //tokens의 row 인덱스의 문자열에 start의 문자를 하나 붙임
						start++;
					}
				}

				else{
					strncpy(tokens[row], start, 1); //tokens의 row 인덱스의 문자열에 start의 문자 하나를 복사
					start += 1;
				}

			}
		  	else if(*end == '*') //찾은 문자열이 * 일때
			{
				isPointer=0; //isPointer 변수를 0으로 초기화

				if(row > 0) //row가 0보다 클때
				{

					for(i = 0; i < DATATYPE_SIZE; i++) { //모든 데이터 타입에 대한 반복을 진행
						if(strstr(tokens[row - 1], datatype[i]) != NULL){ //tokens 의 row-1 인덱스에서 데이터 타입을 찾을 수 있다면
							strcat(tokens[row - 1], "*"); //tokens의 row-1 인덱스에 *을 붙인다
							start += 1;
							isPointer = 1; //isPointer 값을 1로 변경
							break;
						}
					}
					if(isPointer == 1) //isPointerrk 1이면 다음 반복 진행
						continue;
					if(*(start+1) !=0) //start+1 의 문자가 0이면 end를 start의 다음문자로 변경
						end = start + 1;


					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){ //row가 1보다 크고 tokens의 row-2 인덱스의 문자열이 * 이고 row-1 문자열이 모두 *인지 확인하여 참이면
						strncat(tokens[row - 1], start, end - start); //tokens의 row-1 에 end-start 만큼의 문자 갯수를 붙임
						row--; //row 값 감소
					}


					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){ //tokens의 row-1 의 마지막 문자가 문자 또는 숫자라면
						strncat(tokens[row], start, end - start); //tokens의 row에 end-start 길이 만큼의 문자열을 붙임
					}


					else if(strpbrk(tokens[row - 1], op) != NULL){ //tokens의 row-1 에 op 문자가 있으면
						strncat(tokens[row] , start, end - start); //tokens 의 row에 end-start 길이 만큼의 문자열을 붙임

					}
					else
						strncat(tokens[row], start, end - start); //tokens 의 row에 end-start 길이 만큼의 문자열을 붙임

					start += (end - start); //start에 붙인 문자열 만큼 포인터 증가
				}

			 	else if(row == 0) //row가 0일때
				{
					if((end = strpbrk(start + 1, op)) == NULL){ //start+1에서 op 문자열을 찾지 못하면
						strncat(tokens[row], start, 1); //tokens의 row에 start의 문자 1개를 붙임
						start += 1;
					}
					else{
						while(start < end){ //start에서 end까지 반복
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1])) //start의 -1이 공백이면서 tokens의 row의 마지막 값이 문자라면
								return false; //false 리턴
							else if(*start != ' ') //start의 값이 공백이면
								strncat(tokens[row], start, 1); //tokens의 row에 start의 앞문자 한개를 붙임
							start++;
						}
						if(all_star(tokens[row])) //tokens의 row가 모두 *이면 row 값 감소
							row--;

					}
				}
			}
			else if(*end == '(') //end의 문자가 ( 이면
			{
				lcount = 0;
				rcount = 0;
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){ //row 가 0보다 크고 tokens의 row-1 이 &,*이면
					while(*(end + lcount + 1) == '(') //end에 lcount의 값에 1을 더한 것이 (라면
						lcount++; //lcount 증가
					start += lcount; //start에 lcount 만큼 더한다

					end = strpbrk(start + 1, ")"); //end에 start +1에서 )을 찾은 포인터를 대입한다

					if(end == NULL) //찾지 못하면 false 반환
						return false;
					else{
						while(*(end + rcount +1) == ')') //end에서 )가 안나올때까지
							rcount++; //rcount 증가
						end += rcount; //end에 rcount 만큼 더한다

						if(lcount != rcount) //좌우 괄호 갯수가 맞지 않으면 false 반환
							return false;

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){ //row가 1보다 크거나 같고 row의 전전 문자열의 마지막 값이 문자라면
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1); //token의 row-1에 start의 다음 문자에서 end-start-rcount-1 길이 만큼 문자를 붙인다
							row--; //row 감소
							start = end + 1; //start 에 end+1 대입
						}
						else{
							strncat(tokens[row], start, 1); //tokens의 row에 start에서 1만큼 문자를 붙인다
							start += 1; //start 값 증가
						}
					}

				}
				else{
					strncat(tokens[row], start, 1); //tokens의 row에 start에서 1만큼 문자를 붙인다
					start += 1;
				}

			}
			else if(*end == '\"') //\" 를 찾았을때
			{
				end = strpbrk(start + 1, "\""); //end에 start+1에서 \"를 찾은 포인터를 대입

				if(end == NULL) //end가 널이면 false 반환
					return false;

				else{
					strncat(tokens[row], start, end - start + 1); //tokens의 row에 start에서 end-start+1 의 만큼 붙인다
					start = end + 1; //start에 end+1을 대입
				}

			}

			else{

				if(row > 0 && !strcmp(tokens[row - 1], "++")) //row가 0보다 크고 tokens의 row-1이 ++라면 거짓 반환
					return false;


				if(row > 0 && !strcmp(tokens[row - 1], "--")) //row가 0보다 크고 tokens의 row-1이 --라면 거짓 반환
					return false;

				strncat(tokens[row], start, 1); //tokens의 row에 start에서 문자하나를 붙임
				start += 1;//start 증가


				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){ //tokens의 row가 -,+,--,++ 일때



					if(row == 0) //row가 0이라면 row 증가
						row--;


					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ //tokens의 row-1의 마지막이 문자라면

						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL) //tokens의 row-1이 ++,-- 가 아니면
							row--; //row-1
					}
				}
			}
		}
		else{
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) //tokens의 row-1이 전부 *이고 row거 1보다 크고 tokens의 row-2의 마지막 문자가 문자라면
				row--; //row값 감소

			if(all_star(tokens[row - 1]) && row == 1) //row가 1이고 tokens의 row-1이 전부 *이면
				row--; //row 값 감소

			for(i = 0; i < end - start; i++){ //end와 start의 차이만큼 반복
				if(i > 0 && *(start + i) == '.'){ //i가 0보다 크고 start + i가 .이면
					strncat(tokens[row], start + i, 1); //tokens의 row에 start + i 의 문자열의 한글자를 붙임

					while( *(start + i +1) == ' ' && i< end - start ) //start + i + 1 이 공백이고 i 가 end- start 가 될떄 까지 i값 증가
						i++;
				}
				else if(start[i] == ' '){ //start의 i번째 문자가 공백이면
					while(start[i] == ' ') //공백이 아닐때 까지 i 증가
						i++;
					break;
				}
				else
					strncat(tokens[row], start + i, 1); //tokens의 row에 start+i 의 문자열에서 한글자를 붙임
			}

			if(start[0] == ' '){ //start의 첫글자가 공백이면
				start += i; //start 값이 증가하고 다음 반복 수행
				continue;
			}
			start += i; //start의 값을 i만큼 늘림
		}

		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); //tokens의 row에 좌우 공백을 제거한 문자열을 복사

		 if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) //row가 0보다 크고 tokens의 row의 마지막 문자가 숫자 또는 문자이면서 tokens의 row-1의 맨 앞문자가 gcc 또는 데이터 타입이거나 tokens의 row-1의 맨 마지막 문자가 숫자 또는 문자 이거나 tokens의 row-1의 맨 마지막 문자가 . 이면
				&& (is_typeStatement(tokens[row - 1]) == 2
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){

			if(row > 1 && strcmp(tokens[row - 2],"(") == 0) //row가 1보다 크면서 tokens의 row-2가 ( 일때
			{
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0) //tokens의 row-1의 값이 struct, unsigned 가 아니면 false 리턴
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { //row가 1이고 tokens의 row의 마지막 문자가 숫자 또는 문자일때
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) //tokens의 0이 extern, unsigned 가 아니고 tokens의 0이 gcc, 데이터 타입이 아니면 false 반환
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){ //row가 1보다 크고 tokens의 row-1이 gcc, 데이터 타입일때
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0) //tokens의 row-2가 unsigned, extern이 아니라면 거짓 반환
					return false;
			}

		}

		if((row == 0 && !strcmp(tokens[row], "gcc")) ){ //row가 0이고 tokens의 row가 gcc 이면
			clear_tokens(tokens); //tokens의 초기화
			strcpy(tokens[0], str); //tokens의 0에 str 복사
			return 1;// 1반환
		}

		row++; //row 증가
	}

	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) //tokens의 row-1이 전부 *이고 row가 1보다 크고 tokens의 row-2의 끝문자가 숫자 또는 문자가 아니면 row 감소
		row--;
	if(all_star(tokens[row - 1]) && row == 1) //tokens의 row-1이 전부 *이고 row가 1이면 row 감소
		row--;

	for(i = 0; i < strlen(start); i++) //start의 문자열 길이 만큼 반복
	{
		//start의 공백 제거
		if(start[i] == ' ') //start의 i번째 문자가 공백이면
		{
			while(start[i] == ' ')//start에 공백이 안나올때까지
				i++;
			if(start[0]==' ') {//start의 첫문자가 공백이면
				start += i; //start의 i만큼 증가
				i = 0;
			}
			else
				row++; //row 증가

			i--; //i 값 감소
		}
		else
		{
			strncat(tokens[row], start + i, 1); //tokens의 row에 start + i에서 문자를 한개 붙임
			if( start[i] == '.' && i<strlen(start)){ //start의 i번째 문자가 . 이고 i가 start의 문자열 길이보다 작다면
				while(start[i + 1] == ' ' && i < strlen(start)) //start의 i+1 번째 문자가 공백이 아니거나 i가 start 길이 보다 크거나 같아질떄까지 반복
					i++;

			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); //tokens의 row에 tokens의 row의 좌우 공백을 지운 문자열을 복사

		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){ //tokens의 row가 lpthread 이고 row가 0보다 크고 tokens의 row-1이 - 이면
			strcat(tokens[row - 1], tokens[row]); //tokens의 row-1에 row 문자열을 붙임
			memset(tokens[row], 0, sizeof(tokens[row])); //tokens의 row 문자열을 초기화
			row--; //row 감소
		}
	 	else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) //row가 0보다 크고 tokens의 row의 마지막 문자가 숫자 또는 문자이면서 tokens의 row-1 의 첫문자가 gcc, 데이터 타입이거나 tokens의 row-1 의 마지막 문자가 . 일때
				&& (is_typeStatement(tokens[row - 1]) == 2
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){

			if(row > 1 && strcmp(tokens[row-2],"(") == 0) //row가 1보다 크고 tokens의 row-2 가 ( 일때
			{
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0) //tokens의 row-1이 struct, unsigned가 아니면 false 반환
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { //row가 1이고 tokens의 row의 마지막 문자가 숫자 또는 문자 일때
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) //tokens의 0이 extern, unsigned가 아니고 tokens의 0이 gcc, 데이터 타입이 아니면 false 반환
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){ //row 가 1보다 크고 tokens의 row-1이 gcc, 데이터 타입일때
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0) //tokens의 row-2 가 unsigned, extern 이 아니면 false 반환
					return false;
			}
		}
	}


	if(row > 0) //row가 0보다 크면
	{


		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){ //tokens의 0이 #include, include ,struct 이면 tokens 초기화하고 공백을 제거
			clear_tokens(tokens);
			strcpy(tokens[0], remove_extraspace(str));
		}
	}

	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){ //tokens의 0이 gcc, 데이터 타입이거나 extern을 찾을 수 있다면
		for(i = 1; i < TOKEN_CNT; i++){ //TOKEN_CNT 만큼 반복
			if(strcmp(tokens[i],"") == 0) //tokens의 i번째가 아무것도 없으면 중단
				break;

			if(i != TOKEN_CNT -1 ) //만약 마지막 반복이 아니면
				strcat(tokens[0], " "); //tokens의 0에 공백을 붙임
			strcat(tokens[0], tokens[i]); //tokens의 0에 tokens의 i 문자열을 붙임
			memset(tokens[i], 0, sizeof(tokens[i])); //tokens의 i 문자열을 초기화
		}
	}


	while((p_str = find_typeSpecifier(tokens)) != -1){ //tokens의 find_typeSpecifier 함수를 실행한 결과가 -1일때까지 실행
		if(!reset_tokens(p_str, tokens)) //reset_tokens 을 실행하고 false가 리턴되면 false 리턴
			return false;
	}


	while((p_str = find_typeSpecifier2(tokens)) != -1){//tokens의 find_typeSpecifier2 함수를 실행한 결과가 -1일때까지 실행
		if(!reset_tokens(p_str, tokens))//reset_tokens 을 실행하고 false가 리턴되면 false 리턴
			return false;
	}

	return true;
}

node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses)
{
	static int teemp = 0;
	teemp++;
	node *cur = root;
	node *new;
	node *saved_operator;
	node *operator;
	int fstart;
	int i;
	int temp = 0;

	while(1)
	{
		if(strcmp(tokens[*idx], "") == 0) //토큰의 idx 인덱스의 값을 ""과 비교
			break;

		if(!strcmp(tokens[*idx], ")")) //토큰의 idx 값을 )과 비교
			return get_root(cur); //루트 노드를 불러옴

		else if(!strcmp(tokens[*idx], ",")){//토큰의 idx 값을 ,과 비교 면있으면
			return get_root(cur); //루트 노드를 불러옴
		}
		else if(!strcmp(tokens[*idx], "(")) //토큰의 idx 값이 (가 있으면
		{

			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0){//토큰의 idx-1 값이 연산자가 아니고, 토큰의 idx-1 값이 ,가 아니고 idx 가 0보다 크면
				fstart = true; //fstart 를 참으로 바꾸고

				while(1)//무한루프
				{
					*idx += 1; //idx 증가

					if(!strcmp(tokens[*idx], ")")) //토큰의 idx-1 값이 ) 인지 확인
						break;
					new = make_tree(NULL, tokens, idx, parentheses + 1); //make_tree를 새로 호출 하여 new에 입력 받음

					if(new != NULL){ //new가 널이 아니면
						if(fstart == true){ //fstart가 참이면
							cur->child_head = new; //cur 의 child head에 new를 대입
							new->parent = cur; //new의 parent에 cur 을 대입

							fstart = false; //fstart 거짓으로 변경
						}
						else{
							cur->next = new; //fstart가 참이 아니며 cur 의 next에 new 를 대입
							new->prev = cur; //new의 prev에 cur을 대입
						}

						cur = new; //cur에 new를 대입
					}

					if(!strcmp(tokens[*idx], ")")) //tokens의 idx 가 )면 브레이크
						break;
				}
			}
			else{ //토큰의 idx-1 이 연산자가 이거나, 토큰의 idx - 1이 , 이거나, idx가 0보다 작거나 같으면
				*idx += 1; //idx가 1증가
				new = make_tree(NULL, tokens, idx, parentheses + 1);//make_tree를 새로 호출 하여 new에 입력 받음

				if(cur == NULL) //cur이 null 이면
					cur = new; //cur에 new를 대입

				else if(!strcmp(new->name, cur->name)){ //new의 값과 cur의 값이 같을때
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||")
						|| !strcmp(new->name, "&") || !strcmp(new->name, "&&")) //new의 값이 |,||,&,&& 이라면
					{
						cur = get_last_child(cur); //cur의 마지막 자식을 가져옴

						if(new->child_head != NULL){ //new의 자식이 널이 아니라면
							new = new->child_head; //new를 new의 자식으로 변경

							new->parent->child_head = NULL; //new의 부모의 자식을 null로 변경
							new->parent = NULL; //new의 부모를 null로 변경
							new->prev = cur; //new의 prev를 cur로 변경
							cur->next = new; //cur의 next를 new로 변경
						}
					}
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*")) //new의 name이 +,*이면
					{
						i = 0; //i를 0으로 만들고

						while(1) //무한 루프
						{
							if(!strcmp(tokens[*idx + i], "")) //tokens의 idx + i 값이 없는 문자라면 중단
								break;

							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0) //tokens의 iidx + i 연산자가 아니거나 ) 가 아니라면 중단
								break;

							i++;
						}

						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name)) //tokens의 idx + i 값의 연산자 우선순위가 new 의 값보다 작다면
						{
							cur = get_last_child(cur); //cur에 cur의 마지막 자식을 대입
							cur->next = new; //cur의 next가 new
							new->prev = cur;
							cur = new; //cur를 new로 변경
						}
						else //아니라면
						{
							cur = get_last_child(cur); //cur에 cur의 마지막 자식을 대입

							if(new->child_head != NULL){ //new의 자식이 널이 아니라면
								new = new->child_head; //new에

								new->parent->child_head = NULL; //new의 부모 자식 관계를 해제하고
								new->parent = NULL;
								new->prev = cur; //cur의 next로 들어감
								cur->next = new;
							}
						}
					}
					else{
						cur = get_last_child(cur); //cur에 cur의 마지막 자식을 대입
						cur->next = new; //cur의 next에 new를 대입
						new->prev = cur;
						cur = new; //cur를 new로 변경
					}
				}

				else
				{
					cur = get_last_child(cur); //cur에 cur의 마지막 자식을 대입

					cur->next = new; //new를 cur의 next에 대입
					new->prev = cur;

					cur = new; //cur를 new로 변경
				}
			}
		}
		else if(is_operator(tokens[*idx])) {//tokens의 idx 값이 연산자 일때
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&")
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*")) //연산자의 값이 ||, && |, & , +, * 라면
			{
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx])) //cur의 값이 연산자 이고 cur의 값과 tokens의 idx의 값이 일치하면
					operator = cur; //연산자 노드 포인터를 cur로 지정

				else
				{
					new = create_node(tokens[*idx], parentheses); //new에 새로운 노드를 생성
					operator = get_most_high_precedence_node(cur, new); //가장 높은 우선순위를 가진 노드를 가져옴

					if(operator->parent == NULL && operator->prev == NULL){ //operator의 부모와 이전값이 null 일때
						if(get_precedence(operator->name) < get_precedence(new->name)){ //operator 값이 new 의 우선순위보다 높다면
							cur = insert_node(operator, new); //insert_node를 수행
						}

						else if(get_precedence(operator->name) > get_precedence(new->name)) //operator 값이 new 의 우선순위보다 낮다면
						{
							if(operator->child_head != NULL){ //operator의 자식이 널이 아니라면
								operator = get_last_child(operator); //operator에 operator의 마지막 자식을 불러옴
								cur = insert_node(operator, new); //insert_node를 수행
							}
						}
						else
						{
							operator = cur; //operator 에 cur를 대입

							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx])) //operator의 값이 연산자이고 operator의 값이 tokens의 idx와 같다면 중단
									break;

								if(operator->prev != NULL) //operator의 이전값으로 계속 이동
									operator = operator->prev;
								else //operator의 이전값이 없으면 중단
									break;
							}

							if(strcmp(operator->name, tokens[*idx]) != 0) //operator의 값이 tokens의 idx 값과 다르면
								operator = operator->parent; //operator에 operator의 부모 대입

							if(operator != NULL){ //operator가 null이 아닐때
								if(!strcmp(operator->name, tokens[*idx])) //operator의 값이 tokens의 idx와 같다면
									cur = operator; //cur을 operator로 변경
							}
						}
					}

					else
						cur = insert_node(operator, new); //operator의 부모나 이전값이 널이 아니면 insert_node를 수행하고 반환값을 cur에 대입
				}

			}
			else
			{
				new = create_node(tokens[*idx], parentheses); //new에 새로운 노드를 만듬

				if(cur == NULL) //cur가 null 이면
					cur = new; //cur에 new 대입

				else
				{
					operator = get_most_high_precedence_node(cur, new); //operrator에 get_most_high_precedence_node의 반환값을 대입

					if(operator->parentheses > new->parentheses) //만약 operator의 괄호가 new의 괄호도가 크다면
						cur = insert_node(operator, new); //cur에 insert_node를 수행하고 반환된 값을 대입

					else if(operator->parent == NULL && operator->prev ==  NULL){ //operator의 부모가 널이고 operator의 이전값이 널일때

						if(get_precedence(operator->name) > get_precedence(new->name)) //operator의 값의 우선순위가 new 값의 우선순위보다 낮을때
						{
							if(operator->child_head != NULL){ //operator의 자식이 널이 아니라면

								operator = get_last_child(operator); //operator에 operator의 마지막 자식을 가져옴
								cur = insert_node(operator, new); //cur에 insert_node를 수행한 값을 리턴
							}
						}

						else
							cur = insert_node(operator, new); //operator의 값의 우선순위가 new 값의 우선순위보다 낮지 않으면 cur에 insert_node를 수행한 값을 리턴
					}

					else
						cur = insert_node(operator, new); //operator의 부모가 널이지 않거나 operator의 이전값이 널이 아닐때 cur에 insert_node를 수행한 값을 리턴
				}
			}
		}
		else
		{
			new = create_node(tokens[*idx], parentheses); //new에 새로운 노드를 생성

			if(cur == NULL){ //cur가 널이라면
				cur = new; //cur에 new를 대입
			}

			else if(cur->child_head == NULL){ //cur의 자식이 널이 아니라면

				cur->child_head = new; //cur의 자식에 new 대입
				new->parent = cur; //new의 부모에 cur 대입

				cur = new; //cur를 new로 변경
			}
			else{
				cur = get_last_child(cur); //cur에 cur의 마지막 자식 대입

				cur->next = new; //cur의 next에 new 대입
				new->prev = cur; //new의 prew에 cur 대입

				cur = new; //cur에 new 대입
			}
		}

		*idx += 1; //idx 값 증가

	}

	return get_root(cur); //cur의 루트 노드 반환
}

node *change_sibling(node *parent) //자식의 노드 순서를 바꾸는 함수
{
	node *tmp;

	tmp = parent->child_head;

	parent->child_head = parent->child_head->next;
	parent->child_head->parent = parent;
	parent->child_head->prev = NULL;

	parent->child_head->next = tmp;
	parent->child_head->next->prev = parent->child_head;
	parent->child_head->next->next = NULL;
	parent->child_head->next->parent = NULL;

	return parent;
}

node *create_node(char *name, int parentheses)
{
	node *new;

	new = (node *)malloc(sizeof(node));
	new->name = (char *)malloc(sizeof(char) * (strlen(name) + 1)); //new에 동적할당 실시
	strcpy(new->name, name); //name의 값을 new->name에 복사

	new->parentheses = parentheses; //new의 괄호 값을 변경
	new->parent = NULL; //new의 나머지 지정노드를 모두 null로 초기화
	new->child_head = NULL;
	new->prev = NULL;
	new->next = NULL;

	return new; //노드를 반환
}

int get_precedence(char *op)
{
	int i;

	for(i = 2; i < OPERATOR_CNT; i++){
		if(!strcmp(operators[i].operator, op)) //operator_precedence 구조체의 연산자 값과 op가 같다면
			return operators[i].precedence; //operator_precedence 구조체의 우선순위 값을 리턴
	}
	return false;
}

int is_operator(char *op)
{
	int i;

	for(i = 0; i < OPERATOR_CNT; i++)
	{
		if(operators[i].operator == NULL)
			break;
		if(!strcmp(operators[i].operator, op)){ //op와 operator_precedence 구조체의 연산자 값이 같다면 참 리턴
			return true;
		}
	}

	return false;
}

void print(node *cur)
{
	if(cur->child_head != NULL){
		printf("1/");
		print(cur->child_head);
		printf("\n");
	}

	if(cur->next != NULL){
		printf("2/");
		print(cur->next);
		printf("\t");
	}
	printf("%s", cur->name);
}

node *get_operator(node *cur)
{
	if(cur == NULL)
		return cur;

	if(cur->prev != NULL) //cur의 prev가 널이 아니면 계속 prev로 이동
		while(cur->prev != NULL)
			cur = cur->prev;

	return cur->parent; //cur의 parent값 반환
}

node *get_root(node *cur)
{
	if(cur == NULL){ //cur이 널이면 cur 반환
		return cur;
	}

	while(cur->prev != NULL){ //cur의 이전 노드로 계속 이동
		cur = cur->prev;
	}

	if(cur->parent != NULL){ //cur의 부모가 널이 아니면 cur의 부모에 대해 get_root 실행
		cur = get_root(cur->parent);
	}

	return cur; //root 노드 반환
}

node *get_high_precedence_node(node *cur, node *new)
{
	if(is_operator(cur->name)) //cur의 값이 연산자 일때
		if(get_precedence(cur->name) < get_precedence(new->name)) //cur의 연산자 우선순위가 name의 연산자 우선순위 보다 작다면
			return cur; //cur 리턴

	if(cur->prev != NULL){ //cur의 이전 값이 널이 아니면
		while(cur->prev != NULL){
			cur = cur->prev; //계속 cur의 prev 값으로 이동

			return get_high_precedence_node(cur, new); //get_high_precedence_node 를 cur와 new에 대해 수행
		}


		if(cur->parent != NULL) //cur의 부모가 null이 아니라면
			return get_high_precedence_node(cur->parent, new); //cur의 부모와 new에 대해 get_high_precedence_node를 수행
	}

	if(cur->parent == NULL) //cur의 부모가 널이라면
		return cur; //cur 반환
}

node *get_most_high_precedence_node(node *cur, node *new)
{
	node *operator = get_high_precedence_node(cur, new); //new보다 높은 우선순위를 가진 cur의 노드를 가져옴
	node *saved_operator = operator;

	while(1)
	{
		if(saved_operator->parent == NULL)
			break;

		if(saved_operator->prev != NULL)//saved_operator의 이전 노드가 널이 아니라면
			operator = get_high_precedence_node(saved_operator->prev, new); //operator에 new보다 우선순위가 높은 연산자 노드를 반환

		else if(saved_operator->parent != NULL) //saved_operator의 부모가 널이 아니라면
			operator = get_high_precedence_node(saved_operator->parent, new); //operator에 new보다 우선순위가 높은 연산자 노드를 반환

		saved_operator = operator; //saved_operator에 operator 대입
	}

	return saved_operator; //saved_operator반환
}

node *insert_node(node *old, node *new)
{
	if(old->prev != NULL){ //old의 이전값이 널이 아니라면
		new->prev = old->prev; //new의 이전값에 old의 이전값을 넣는다
		old->prev->next = new; //old의 이전값의 다음값이 new를 가리킨
		old->prev = NULL; //old의 이전값이 널로 변경된다
	}

	new->child_head = old; //new의 자식에 old를 넣는다
	old->parent = new; //old의 부모가 new가 된다

	return new; //new 반환
}

node *get_last_child(node *cur)
{
	if(cur->child_head != NULL) //cur의 자식이 널이 아니면 cur의 cur의 자식으로 변경
		cur = cur->child_head;

	while(cur->next != NULL) //cur의 next가 널이 아닐때까지 next로 이동
		cur = cur->next;

	return cur; //cur 반환
}

int get_sibling_cnt(node *cur)
{
	int i = 0;

	while(cur->prev != NULL)
		cur = cur->prev;

	while(cur->next != NULL){
		cur = cur->next;
		i++;
	}

	return i;
}

void free_node(node *cur)
{
	if(cur->child_head != NULL)
		free_node(cur->child_head);

	if(cur->next != NULL)
		free_node(cur->next);

	if(cur != NULL){
		cur->prev = NULL;
		cur->next = NULL;
		cur->parent = NULL;
		cur->child_head = NULL;
		free(cur);
	}
}


int is_character(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_typeStatement(char *str)
{
	char *start;
	char str2[BUFLEN] = {0};
	char tmp[BUFLEN] = {0};
	char tmp2[BUFLEN] = {0};
	int i;

	start = str;
	strncpy(str2,str,strlen(str)); //str2에 str의 str의 길이 만큼 복사
	remove_space(str2); //str2의 공백 제거

	while(start[0] == ' ') //왼쪽의 공백이 없어질때까지 start값 증가
		start += 1;

	if(strstr(str2, "gcc") != NULL) //str2에 gcc가 있다면
	{
		strncpy(tmp2, start, strlen("gcc"));//start의 처음이 gcc이면 0 아니면 2 리턴
		if(strcmp(tmp2,"gcc") != 0)
			return 0;
		else
			return 2;
	}

	for(i = 0; i < DATATYPE_SIZE; i++) //데이터 타입 사이즈 만큼 반복
	{
		if(strstr(str2,datatype[i]) != NULL) //str2의 앞에 데이터 타입이 있으면 2 아니면 0 리턴
		{
			strncpy(tmp, str2, strlen(datatype[i]));
			strncpy(tmp2, start, strlen(datatype[i]));

			if(strcmp(tmp, datatype[i]) == 0)
				if(strcmp(tmp, tmp2) != 0)
					return 0;
				else
					return 2;
		}

	}
	return 1;

}

int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]) //tokens에서 바로 앞뒤에 괄호가 있고 뒤에 연산자 또는 문자가 나오는 부분을 찾는 함수
{
	int i, j;

	for(i = 0; i < TOKEN_CNT; i++)
	{
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			if(strstr(tokens[i], datatype[j]) != NULL && i > 0)
			{
				if(!strcmp(tokens[i - 1], "(") && !strcmp(tokens[i + 1], ")")
						&& (tokens[i + 2][0] == '&' || tokens[i + 2][0] == '*'
							|| tokens[i + 2][0] == ')' || tokens[i + 2][0] == '('
							|| tokens[i + 2][0] == '-' || tokens[i + 2][0] == '+'
							|| is_character(tokens[i + 2][0])))
					return i;
			}
		}
	}
	return -1;
}

int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]) //구조체가 선언된 부분을 찾는 함수
{
    int i, j;


    for(i = 0; i < TOKEN_CNT; i++)
    {
        for(j = 0; j < DATATYPE_SIZE; j++)
        {
            if(!strcmp(tokens[i], "struct") && (i+1) <= TOKEN_CNT && is_character(tokens[i + 1][strlen(tokens[i + 1]) - 1]))
                    return i;
        }
    }
    return -1;
}

int all_star(char *str)
{
	int i;
	int length= strlen(str); //str의 길이 파악

 	if(length == 0)
		return 0;

	for(i = 0; i < length; i++)
		if(str[i] != '*') //i 인덱스의 문자가 *이 아니면 0 리턴
			return 0;
	return 1;

}

int all_character(char *str)
{
	int i;

	for(i = 0; i < strlen(str); i++)
		if(is_character(str[i]))
			return 1;
	return 0;

}

int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN])
{
	int i;
	int j = start - 1;
	int lcount = 0, rcount = 0;
	int sub_lcount = 0, sub_rcount = 0;

	if(start > -1){ // start가 -1보다 클때
		if(!strcmp(tokens[start], "struct")) { //tokens의 start번째 문자열에 struct 이면
			strcat(tokens[start], " "); //뒤에 공백을 붙인다
			strcat(tokens[start], tokens[start+1]); //tokens의 start에 다음 문자열을 붙인다

			for(i = start + 1; i < TOKEN_CNT - 1; i++){ //tokens의 start 다음 인덱스를 한칸 씩 앞으로 당김
				strcpy(tokens[i], tokens[i + 1]);
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

		else if(!strcmp(tokens[start], "unsigned") && strcmp(tokens[start+1], ")") != 0) { //tokens의 start가 unsigned 이고 다음이 ) 면
			strcat(tokens[start], " "); //공백을 붙인다
			strcat(tokens[start], tokens[start + 1]); // )를 붙인다
			strcat(tokens[start], tokens[start + 2]); // unsigned ) 에 다음 문자열까지 붙인다

			for(i = start + 1; i < TOKEN_CNT - 1; i++){ //tokens의 start 다음 인덱스를 한칸 씩 앞으로 당김
				strcpy(tokens[i], tokens[i + 1]);
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

     		j = start + 1; //j를 start+1로 변경
        	while(!strcmp(tokens[j], ")")){ //)가 안나올떄까지 반복
                	rcount ++; //rcount 값 증가
                	if(j==TOKEN_CNT) //j가 TOKEN_CNT와 같아지면 중단
                        	break;
                	j++;
        	}

		j = start - 1; //j를 start-1로 변경
		while(!strcmp(tokens[j], "(")){ //(가 나오지 않을때까지 앞으로 반복
        	        lcount ++; //lcount 값 증가
                	if(j == 0) //j가 0 이면 종료
                        	break;
               		j--;
		}
		if( (j!=0 && is_character(tokens[j][strlen(tokens[j])-1]) ) || j==0) //j가 0이 아니고, j번째 문자열의 마지막 문자가 숫자 또는 문자 이거나 j가 0이면
					lcount = rcount; //lcount에 rcount 값을 대입

		if(lcount != rcount ) //lcount 와 rcount 가 같지 않으면 false 반환
			return false;

		if( (start - lcount) >0 && !strcmp(tokens[start - lcount - 1], "sizeof")){ //start - lcount 가 0보다 크고 tokens의 start - lcount - 1 번째 문자가 sizeof 라면 참 반환
			return true;
		}

		else if((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start+1], ")")) { //tokens의 start가 unsigned 또는 struct이고 다음문자가 ) 이면
			strcat(tokens[start - lcount], tokens[start]); //tokens의 start- lcount에 start 번째 문자열을 붙인다
			strcat(tokens[start - lcount], tokens[start + 1]); //tokens의 start- lcount에 start +1 문자열을 붙인다
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]); //tokens의 start - lcount + 1에 start + rcount 번째의 문자열을 복사한다

			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount; i++) { //start - lcount + 1번째 부터 TOKEN_CNT - lcount -rcount번쨰까지 문자열을 앞으로 옮긴다
				strcpy(tokens[i], tokens[i + lcount + rcount]);
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0]));
			}


		}
 		else{
			if(tokens[start + 2][0] == '('){ //tokens의 start + 2가 ( 이면
				j = start + 2; //j를 start + 2로 변경
				while(!strcmp(tokens[j], "(")){ //tokens에서 (가 나오지 않을때 까지 j값을 증가시키면서 체크
					sub_lcount++; //sub_lcount 값 증가
					j++;
				}
				if(!strcmp(tokens[j + 1],")")){ //tokens의 j+1 값이 ) 라면
					j = j + 1; //j값을 1 증가 시킨다
					while(!strcmp(tokens[j], ")")){ //tokens에서 )가 나오지 않을때까지 j 값을 증가시키면서 체크
						sub_rcount++; //sub_rcount 증가
						j++;
					}
				}
				else//tokens의 j+1 값이 ) 가 아니라면 false 리턴
					return false;

				if(sub_lcount != sub_rcount) //괄호수가 맞지 않으면 false 리턴
					return false;

				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]); //tokens의 start + 2
				for(int i = start + 3; i<TOKEN_CNT; i++) //start + 3 부터 TOKEN_CNT 까지 tokens의 값을 0 으로 초기화함
					memset(tokens[i], 0, sizeof(tokens[0]));

			}
			strcat(tokens[start - lcount], tokens[start]); //tokens의 start - lcount 에 tokens의 start의 값을 붙임
			strcat(tokens[start - lcount], tokens[start + 1]);//tokens의 start - lcount 에 tokens의 start + 1의 값을 붙임
			strcat(tokens[start - lcount], tokens[start + rcount + 1]);//tokens의 start - lcount 에 tokens의 start + rcount + 1의 값을 붙임

			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount -1; i++) { //start - lcount + 1 에서 TOKEN_CNT - lcount -rcount -1 까지 반복 하여 문자열을 한칸씩 앞으로 옮김
				strcpy(tokens[i], tokens[i + lcount + rcount +1]);
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0]));

			}
		}
	}
	return true;
}

void clear_tokens(char tokens[TOKEN_CNT][MINLEN])
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i]));
}

char *rtrim(char *_str)
{
	char tmp[BUFLEN];
	char *end;

	strcpy(tmp, _str); //tmp에 _str 문자열 복사
	end = tmp + strlen(tmp) - 1;
	while(end != _str && isspace(*end)) //end의 주소값이 _str의 주소값과 다르고 end의 문자가 공백일때 end 값을 줄이며 계속 반복
		--end;

	*(end + 1) = '\0'; //end의 주소값 + 1을 널문자로 변경
	_str = tmp; //_str에 tmp의 주소값을 대입
	return _str;
}

char *ltrim(char *_str)
{
	char *start = _str;

	while(*start != '\0' && isspace(*start)) //start의 문자가 널문자가 아니고 공백이면 start의 주소값 증가
		++start;
	_str = start; //start의 주소값을 _str에 대입
	return _str;
}

char* remove_extraspace(char *str)
{
	int i;
	char *str2 = (char*)malloc(sizeof(char) * BUFLEN);
	char *start, *end;
	char temp[BUFLEN] = "";
	int position;

	if(strstr(str,"include<")!=NULL){ //str에 include< 가 있다면
		start = str;
		end = strpbrk(str, "<"); //end에 <을 찾아 대입
		position = end - start;

		strncat(temp, str, position); //temp에 str의 positon 길이 만큼 붙임
		strncat(temp, " ", 1); //temp에 공백을 붙임
		strncat(temp, str + position, strlen(str) - position + 1); //temp에 str + positon 포인터에 str의 길이 -position +1 길이 만큼 붙임

		str = temp; //str에 temp 대입
	}

	for(i = 0; i < strlen(str); i++) //str의 길이만큼 반복
	{
		if(str[i] ==' ') //str에 공백이 있을때
		{
			if(i == 0 && str[0] ==' ') //i가 0이고 str[0]이 공백이면
				while(str[i + 1] == ' ') //공백이 안나올때까지 i 증가
					i++;
			else{
				if(i > 0 && str[i - 1] != ' ') //i가 0보다 크고 str의 i-1 문자가 공백이 아니면
					str2[strlen(str2)] = str[i]; //str2의 마지막 인덱스에 str의 i번째 문자를 대입
				while(str[i + 1] == ' ') //str에 공백이 안나올때까지 i 증가
					i++;
			}
		}
		else
			str2[strlen(str2)] = str[i]; //str2의 마지막 문자에 str의 i번째 문자를 대입
	}

	return str2;
}



void remove_space(char *str) //문자열을 받아 공백을 제거하는 함수
{
	char* i = str;
	char* j = str;

	while(*j != 0)
	{
		*i = *j++;
		if(*i != ' ') //공백을 만나면 포인터를 한칸 증가시킴
			i++;
	}
	*i = 0;
}

int check_brackets(char *str)
{
	char *start = str;
	int lcount = 0, rcount = 0;

	while(1){
		if((start = strpbrk(start, "()")) != NULL){
			if(*(start) == '(')
				lcount++;
			else
				rcount++;

			start += 1;
		}
		else
			break;
	}

	if(lcount != rcount)
		return 0;
	else
		return 1;
}

int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]) //tokens의 갯수를 확인해주는 함수
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		if(!strcmp(tokens[i], ""))
			break;

	return i;
}
