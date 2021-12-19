#ifndef BLANK_H_
#define BLANK_H_

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef BUFLEN
	#define BUFLEN 1024
#endif

#define OPERATOR_CNT 24
#define DATATYPE_SIZE 35
#define MINLEN 64
#define TOKEN_CNT 50

typedef struct node{
	int parentheses;
	char *name;
	struct node *parent;
	struct node *child_head;
	struct node *prev;
	struct node *next;
}node;

typedef struct operator_precedence{
	char *operator;
	int precedence;
}operator_precedence;

void compare_tree(node *root1,  node *root2, int *result); //트리를 비교하는 함수
node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses); //연산 트리를 만드는 함수
node *change_sibling(node *parent); //자식의 next와 자식의 위치를 변경하는 함수
node *create_node(char *name, int parentheses); //새로운 노드를 만들어 주는 함수
int get_precedence(char *op); //op의 연산자 우선순위를 가져오는 함수
int is_operator(char *op); //op가 연산자인지 확인하는 함수
void print(node *cur); //tree를 출력하는 함수
node *get_operator(node *cur); //cur의 연산자 노드를 반환 하는 함수
node *get_root(node *cur); //cur의 루트 노드 반환하는 함수
node *get_high_precedence_node(node *cur, node *new); //cur의 new를 비교해 new보다 연산자 우선순위가 높은 노드를 반환하는 함수
node *get_most_high_precedence_node(node *cur, node *new); //가장 높은 우선순위의 연산자를 가져오는 함수
node *insert_node(node *old, node *new); //new가 old의 자리를 대신하고 old의 부모가 된다
node *get_last_child(node *cur); //cur의 마지막 자식을 가져오는 함수
void free_node(node *cur); //cur 트리의 모든 값을 비우는 함수
int get_sibling_cnt(node *cur); //노드의 next 갯수를 세어주는 함수

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN]); //식을 tokenize 하는 함수
int is_typeStatement(char *str); //데이터 타입, gcc에 대한 문법이 맞는지 확인
int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]);//tokens에서 바로 앞뒤에 괄호가 있고 뒤에 연산자 또는 문자가 나오는 부분을 찾는 함수
int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]);//구조체가 선언된 부분을 찾는 함수
int is_character(char c); //문자가 숫자인지 영어인지 확인하는 함수
int all_star(char *str); //문자열이 모두 *로 되어있는지 확인하는 함수
int all_character(char *str); //문자열의 구성원이 모두 문자 또는 숫자인지 확인하는 함수
int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]); //tokens의 설정을 변경하는 함수
void clear_tokens(char tokens[TOKEN_CNT][MINLEN]); //tokens를 초기화하는 함수
int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]);
char *rtrim(char *_str); //문자열의 우측 공백을 제거하는 함수
char *ltrim(char *_str); //문자열의 좌측 공백을 제거하는 함수
void remove_space(char *str); //문자열의 공백을 제거하는 함수
int check_brackets(char *str); //괄호의 갯수가 맞는지 확인하는 함수
char* remove_extraspace(char *str);//공백을 제거해주는 함수

#endif
