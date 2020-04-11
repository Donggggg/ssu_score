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
		, "rlim_t", "jmp_buf", "sig_atomic_t", "clock_t", "struct"}; // 데이터타입 모음


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
}; // 연산자 밑 우선순위를 저장하는 배열 

void compare_tree(node *root1,  node *root2, int *result) // 트리를 비교해주는 함수 
{
	node *tmp;
	int cnt1, cnt2;

	if(root1 == NULL || root2 == NULL){ // 1과 2가 둘다 널이면 
		*result = false;
		return;
	}

	if(!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")){ // 1의 요소가 부등호류 이면 한뱡항으로 바꿔줌
		if(strcmp(root1->name, root2->name) != 0){

			if(!strncmp(root2->name, "<", 1))
				strncpy(root2->name, ">", 1);

			else if(!strncmp(root2->name, ">", 1))
				strncpy(root2->name, "<", 1);

			else if(!strncmp(root2->name, "<=", 2))
				strncpy(root2->name, ">=", 2);

			else if(!strncmp(root2->name, ">=", 2))
				strncpy(root2->name, "<=", 2);

			root2 = change_sibling(root2); // 형제도 교체 
		}
	}

	if(strcmp(root1->name, root2->name) != 0){ // 1과 2가 다르면 
		*result = false;
		return;
	}

	if((root1->child_head != NULL && root2->child_head == NULL) 
			|| (root1->child_head == NULL && root2->child_head != NULL)){ // 형제의 구성이 다르면 
		*result = false;
		return;
	}

	else if(root1->child_head != NULL){ // 1의 자식이 있으면
		if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){ // 1과 2의 자식수가 틀리면 
			*result = false;
			return;
		}

		if(!strcmp(root1->name, "==") || !strcmp(root1->name, "!=")) // 1의 내용이 ==나 !=이면
		{
			compare_tree(root1->child_head, root2->child_head, result); // 다시 비교 

			if(*result == false) // 결과가 false면  
			{
				*result = true; //true로 바꾸고
				root2 = change_sibling(root2); //형제를 바꾸고 
				compare_tree(root1->child_head, root2->child_head, result);// 다시 비교
			}
		}
		else if(!strcmp(root1->name, "+") || !strcmp(root1->name, "*")
				|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
				|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&"))
		{ // 양변이 상관 없는 연산자일 경우
			if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){ // 자식의 수가 다르면 
				*result = false; // 
				return;
			}

			tmp = root2->child_head; // 2의 자식을 tmp로 저장

			while(tmp->prev != NULL) // tmp의 마지막으로 이동 
				tmp = tmp->prev;

			while(tmp != NULL) // tmp의 마지막이 널이아니면 
			{
				compare_tree(root1->child_head, tmp, result); // 루트1의 자식과 tmp를 비교 

				if(*result == true) // true면 break
					break;
				else{ 
					if(tmp->next != NULL) // 널이아니면 
						*result = true;
					tmp = tmp->next;
				}
			}
		}
		else{ //그외의 경우 
			compare_tree(root1->child_head, root2->child_head, result);
		}
	}	


	if(root1->next != NULL){ // 1의 다음이 널이 아니면 

		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)){ // 1과 2의 자식 수가 다르면 
			*result = false;
			return;
		}

		if(*result == true)
		{
			tmp = get_operator(root1); // root1의 부모를 얻음

			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*")
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&"))
			{	//양 변의 인자가 상관 없는 연산자인 경우
				tmp = root2; // tmp에 2를 저장

				while(tmp->prev != NULL) // tmp의 헤드까지
					tmp = tmp->prev;

				while(tmp != NULL) // tmp가 널이 아닐때까지 
				{
					compare_tree(root1->next, tmp, result); // 1의 다음과 tmp를 비교

					if(*result == true)
						break;
					else{
						if(tmp->next != NULL)
							*result = true;
						tmp = tmp->next;
					}
				}
			}

			else
				compare_tree(root1->next, root2->next, result);
		}
	}
}

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN]) // 문자열을 토크나이징하는 함수
{
	char *start, *end; // 문자열을 탐색하기 위함
	char tmp[BUFLEN]; 
	char str2[BUFLEN];
	char *op = "(),;><=!|&^/+-*\""; // 연산자모음 문자열 
	int row = 0; // 현재 토큰 위치
	int i;
	int isPointer; // 포인터인지 체크하는 변수
	int lcount, rcount; // 좌,우 괄호 체크 변수
	int p_str; 

	clear_tokens(tokens); // 토큰리스트 초기화

	start = str; // 문자열의 시작 주소 저장 

	if(is_typeStatement(str) == 0) // 데이터타입인지 체크
		return false;	

	while(1)
	{
		if((end = strpbrk(start, op)) == NULL) // 연산기호가 없으면 break;
		break;

		if(start == end){ // 연산기호가 맨 앞에 있으면

			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)){ // '--', '++'이면
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4)) // '----', '++++'이면 false
					return false;

				if(is_character(*ltrim(start + 2))){ // 바로 다음 인자가 문자이면
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])) // 토큰이 1개 이상이고 앞 토큰의 마지막 인자가 문자이면 false
						return false; 

					end = strpbrk(start + 2, op); // 다음 연산자전까지 end 위치 변경
					if(end == NULL) // end가 문자열 끝을 초과하면
						end = &str[strlen(str)]; // str의 마지막 주소로 end 위치 변경 
					while(start < end) { // start부터 end까지 
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))  // start전 문자가 공백이고 현재 토큰의 마지막 인자가 문자이면 false
							return false;
						else if(*start != ' ') // 공백이 아니면 토큰에 붙임
							strncat(tokens[row], start, 1); // 현재 토큰에 start 1바이트를 붙임 
						start++; // start 이동
					}
				}

				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ // 앞 토큰의 마지막 인자가 문자이면
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL) // 앞 토큰이 전위연산자이면 false
						return false;

					memset(tmp, 0, sizeof(tmp)); // tmp 메모리셋 
					strncpy(tmp, start, 2); // tmp에 start 2바이트 복사
					strcat(tokens[row - 1], tmp); // tmp에 앞 토큰 내용 복사 
					start += 2; // start 2바이트 이동
					row--; // 현재토큰개수감소
				}
				else{ // 그 외의 경우 
					memset(tmp, 0, sizeof(tmp)); // 토큰에 2바이트만 저장 
					strncpy(tmp, start, 2);
					strcat(tokens[row], tmp);
					start += 2;
				}
			}

			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2)
					|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
					|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
					|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
					|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){ //비교 연산자면

				strncpy(tokens[row], start, 2); // 2바이트 저장
				start += 2; // 2바이트 이동 
			}
			else if(!strncmp(start, "->", 2)) // 구조체포인터 변수 접근자이면
			{
				end = strpbrk(start + 2, op); // 다음 문자열 끝을 end로

				if(end == NULL) // end가 초과하면 
					end = &str[strlen(str)]; // end를 str의 끝으로 지정 

				while(start < end){ // end 주소까지 
					if(*start != ' ') // 공백이 아니면 
						strncat(tokens[row - 1], start, 1); // 앞 토큰에 1바이트 이어붙이기
					start++; // start 이동
				}
				row--; // 현재토큰개수감소
			}
			else if(*end == '&') // '&'이면
			{
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){ // 첫 토큰이거나 앞토큰에 연산자가 없으면
					end = strpbrk(start + 1, op); // 1바이트 후에 연산자 위치를 end에 저장 
					if(end == NULL) // end가 널을 초과하면
						end = &str[strlen(str)]; // str의 마지막으로 end변경

					strncat(tokens[row], start, 1); // 현재 토큰에 start 1바이트('&') 붙임
					start++;// start 이동 

					while(start < end){ // end 주소까지 
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&') // start 1바이트 전이 공백이고 현재토큰의 마지막 문자가 '&'이면 false 
							return false;
						else if(*start != ' ') // 공백이 아니면 
							strncat(tokens[row], start, 1); // 1바이트 붙임 
						start++;
					}
				}
				else{ // 그 외의 경우 
					strncpy(tokens[row], start, 1); // 1바이트 붙이고 
					start += 1; // 1바이트 이동 
				}

			}
			else if(*end == '*') // '*'이면
			{
				isPointer=0; // isPointer 0으로 변경

				if(row > 0) // 토큰이 1개 이상이면 
				{
					for(i = 0; i < DATATYPE_SIZE; i++) { // datatype의 개수만큼 
						if(strstr(tokens[row - 1], datatype[i]) != NULL){ // 앞 토큰에 datatype이 있으면 
							strcat(tokens[row - 1], "*"); // 앞 토큰에 '*' 삽입 
							start += 1;	 // start 1바이트 이동 
							isPointer = 1; // isPointer 1로 변경 
							break;
						}
					}
					if(isPointer == 1) // 1이면
						continue; 
					if(*(start+1) !=0) // 다음 인자가 널문자가 아니면 
						end = start + 1; // end를 한칸 이동 


					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){ // 토큰이 2개 이상이고 2개 앞 토큰이 '*'이고  앞토큰이 모두 '*'이면 
						strncat(tokens[row - 1], start, end - start); // 앞토큰에 
						row--; // 토큰 개수 감소
					}
					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){  // 앞토큰의 마지막 인자가 문자면 
						strncat(tokens[row], start, end - start); // 현재 토큰에 end - start만큼 이어붙임
					}
					else if(strpbrk(tokens[row - 1], op) != NULL){	// 앞토큰에 연산자가 있으면 
						strncat(tokens[row] , start, end - start); // 현재 토큰에 end - start만큼 이어붙임
					} // 그 외
					else
						strncat(tokens[row], start, end - start); // 현재 토큰에 end - start만큼 이어붙임

					start += (end - start); // start를 end-start만큼 이동
				}

				else if(row == 0) // 첫 토큰이면 
				{
					if((end = strpbrk(start + 1, op)) == NULL){ // 다음 연산자가 없으면 
						strncat(tokens[row], start, 1); // 토큰에 1바이트 '*' 붙이고
						start += 1; // start 1이동
					}
					else{ // 연산자가 있으면
						while(start < end){ // start를 end까지 
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1])) // start전이 공백이고 현재토큰의 마지막 인자가 문자이면 false
								return false;
							else if(*start != ' ') // start가 공백이 아니면 
								strncat(tokens[row], start, 1); // 1바이트 토큰에 붙여줌 
							start++; // start 이동 
						}
						if(all_star(tokens[row])) // 현재토큰이 모두 '*'로 이루어졌으면 토큰개수감소 
							row--;

					}
				}
			}
			else if(*end == '(') // 여는 괄호면
			{
				lcount = 0; 
				rcount = 0;
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){ // 토큰이 1개 이상이고 앞 토큰이 '&'이거나 '*'이면 
					while(*(end + lcount + 1) == '(') // 괄호 내부에 '('이 또 있으면 
						lcount++; // 여는괄호 카운트 증가
					start += lcount; // 가장 안쪽 여는괄호로 start 이동

					end = strpbrk(start + 1, ")"); // 가장 안쪽 닫는 괄호로 end 이동

					if(end == NULL) // 닫는 괄호가 없으면 false
						return false;
					else{ 
						while(*(end + rcount +1) == ')') // 닫는 괄호 개수 체크 
							rcount++;
						end += rcount;

						if(lcount != rcount) // 개수가 안맞으면 false 
							return false;

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){  // 토큰이 2개 이상이고 2개 앞토큰의 마지막 인자가 문자가 아니거나 토큰이 1개이면 
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1); // 앞토큰에 start + 1부터 닫는괄호 전까지 넣는다 
							row--; // 토큰개수감소
							start = end + 1; // 가장 바깥쪽 닫는괄호 다음으로 이동 
						}
						else{ // 현재 토큰에 1바이트 붙임
							strncat(tokens[row], start, 1);
							start += 1;
						}
					}

				}
				else{ // 그외엔 '(' 붙임 
					strncat(tokens[row], start, 1);
					start += 1;
				}

			}
			else if(*end == '\"') // 따옴표면 
			{
				end = strpbrk(start + 1, "\""); // 다음 따옴표의 위치를 end로

				if(end == NULL) // 다음 따옴표가 없으면 false 
					return false;

				else{ // 현재 토큰에 따옴표사이의 내용을 붙임 
					strncat(tokens[row], start, end - start + 1);
					start = end + 1;
				}

			}

			else{ // 그 외의 경우면

				if(row > 0 && !strcmp(tokens[row - 1], "++")) // 토큰이 1개이상이고 앞토큰이 ++이면 false
					return false;


				if(row > 0 && !strcmp(tokens[row - 1], "--")) // 토큰이 1개이상이고 앞토큰이 --이면 false
					return false;

				strncat(tokens[row], start, 1); // 현재 토큰에 1바이트 붙임 
				start += 1; 


				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){ // 

					if(row == 0) // 첫토큰이면 row감소
						row--;

					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ // 앞 토큰의 마지막 인자가 문자가 아니면 

						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL) // 앞토큰에 ++이 없고 --가 없으면 row감소
							row--;
					}
				}
			}
		}
		else{ // start와 end가 다르면(보통 문자열)
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) // 포인터인지검사(row가 2이상)
				row--;				

			if(all_star(tokens[row - 1]) && row == 1)   // 포인터인지검사 (row가 1)
				row--;	

			for(i = 0; i < end - start; i++){ // 토큰 길이만큼
				if(i > 0 && *(start + i) == '.'){ // 구조체접근이면
					strncat(tokens[row], start + i, 1); // '.'을 바로 붙여줌

					while( *(start + i + 1) == ' ' && i< end - start )
						i++; 
				}
				else if(start[i] == ' '){ // 스페이스면 제거
					while(start[i] == ' ')
						i++;
					break;
				}
				else // 그 외면 토큰에 붙임
					strncat(tokens[row], start + i, 1);
			}

			if(start[0] == ' '){
				start += i;
				continue;
			}
			start += i; // 토큰이 된만큼 start 좌표 옮김
		}

		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); // trim하여 다시 저장

		// 토큰이 1개 이상이고 현재토큰의 마지막이 문자이고 앞토큰이 데이터타입이거나
		// 앞토큰의 마지막 인자가 문자이거나 앞토큰의 마지막문자가 '.'이면
		if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){
			if(row > 1 && strcmp(tokens[row - 2],"(") == 0) // 토큰이 2개 이상 있고 2개 앞토큰이 여는 괄호일 때
			{
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0)// 앞의 토큰이 struct과 unsigned가 아니면 false
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { // 토큰이 한개이고 현재토큰의 마지막 인자가 문자일 때
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	// 첫토큰이 extern이 아니고 unsigned가 아니고 데이터타입이 아니면 false 
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){ // 토큰이 2개 이상 있고 앞의토큰이 데이터타입일 때
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false; // 두개앞토큰이 unsigned가아니고 extern도 아니면 false
			}

		} 
		if((row == 0 && !strcmp(tokens[row], "gcc")) ){ // 첫 토큰이 gcc이면
			clear_tokens(tokens); // 토큰을 비워주고
			strcpy(tokens[0], str);	//첫토큰에 문장 전체를 넣어주고 종료 
			return 1;
		} 
		row++;
	} // while문 끝

	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))  // 앞토큰이 별이고 2개앞 토큰마지막이 문자가 아니면
		row--;				
	if(all_star(tokens[row - 1]) && row == 1)   //앞토큰이 별이면
		row--;	

	for(i = 0; i < strlen(start); i++) //  start에 남은 문자처리
	{
		if(start[i] == ' ') // 공백이면 
		{
			while(start[i] == ' ') // 공백이 아닐때까지 i증가 
				i++;
			if(start[0]==' ') { // 첫인자가 공백이면
				start += i; // start의 시작위치 i까지 옮기고
				i = 0; // i를 0으로 
			}
			else
				row++; // 토큰개수 증가

			i--; 
		} 
		else 
		{
			strncat(tokens[row], start + i, 1); // 현재토큰에 1바이트 붙임
			if( start[i] == '.' && i<strlen(start)){ // 만약 '.'이고 위치가 start를 넘지 않았으면 
				while(start[i + 1] == ' ' && i < strlen(start)) // 다음문자가 공백이 아닐때까지 i증가
					i++;
			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); // 앞뒤 공백제거 후 토큰에 추가 

		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){  // -lpthread 옵션이 있으면
			strcat(tokens[row - 1], tokens[row]); // 두개를 붙여서 한 토큰에 넣고
			memset(tokens[row], 0, sizeof(tokens[row]));
			row--; // 토큰 개수감소
		}
		else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1])  
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){ // 토큰이 1개 이상이고 현재토큰의 마지막 인자가 문자이고 데이터탑이거나 앞토큰의 마지막 인자가 문자이거나 점일 때

			if(row > 1 && strcmp(tokens[row-2],"(") == 0) // 토큰이 2개이상이고 2개앞 토큰이 여는 괄호면
			{
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0) // 앞토큰이 struct가 아니거나 unsigned가 아니면 false
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { // 토큰이 2개이고 현재 토큰의 마지막 인자가 문자이면 
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	 // 0번째 토큰이 extern이 아니거나 unsigned가 아니거나 데이터타입이 아니면 false
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){ // 토큰이 2개이상이고 앞 토큰이 데이터타입이면
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false; // 두개 앞 토큰이 unsigned가 아니고 extern도 아니면 false
			}
		} 
	}

	if(row > 0) // 토큰이 1개이상이면
	{

		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){  // 첫번째 토큰이 #include이거나 include이거나 struct이면
			clear_tokens(tokens); //토큰을 정리하고
			strcpy(tokens[0], remove_extraspace(str)); // 과도한 공백 제거 
		}
	}

	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){ // 첫 토큰이 데이터타입이거나 extern이면
		for(i = 1; i < TOKEN_CNT; i++){ // 토큰개수만큼순회
			if(strcmp(tokens[i],"") == 0) //토큰내에 정보가 없으면 break
				break;		       
			if(i != TOKEN_CNT -1 ) // 토큰의 마지막이 아니면
				strcat(tokens[0], " "); // 첫토큰에 공백을 붙임
			strcat(tokens[0], tokens[i]); // 첫토큰에 i번째 토큰을 붙이고
			memset(tokens[i], 0, sizeof(tokens[i])); // i번째 토큰 메모리초기화
		}
	}

	while((p_str = find_typeSpecifier(tokens)) != -1){  // cast 연산자일때 해당 토큰을 찾고 토큰재정렬
		if(!reset_tokens(p_str, tokens))
			return false;
	}

	while((p_str = find_typeSpecifier2(tokens)) != -1){  // struct일때 해당 토큰을 찾고 토큰재정렬
		if(!reset_tokens(p_str, tokens))
			return false;   
	} 

	return true;
}

node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses) //주어진 노드의 트리 생성
{
	node *cur = root; // 현재를 가리키는 node
	node *new;
	node *saved_operator;
	node *operator;
	int fstart;
	int i;

	while(1) // 토큰 탐색
	{
		if(strcmp(tokens[*idx], "") == 0) // 토큰이 비었으면 break
			break;

		if(!strcmp(tokens[*idx], ")")) // 토큰 배열에서 닫는 괄호를 만나면, 괄호내용을 포함하고 있는 부모로 돌아감
			return get_root(cur);

		else if(!strcmp(tokens[*idx], ",")) // 토큰 배열에서 ,를 만나면 부모로 돌아감
			return get_root(cur);

		else if(!strcmp(tokens[*idx], "(")) // 여는 괄호면
		{

			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0){ // 앞 토큰이 연산자가 아니고 쉼표가 아니면 (함수)
				fstart = true;

				while(1)
				{
					*idx += 1; //다음 토큰으로 이동

					if(!strcmp(tokens[*idx], ")")) // 닫는괄호면 즉시 탈출 
						break;

					new = make_tree(NULL, tokens, idx, parentheses + 1); // 자식 트리 생성 

					if(new != NULL){ // new가 생성되었으면 
						if(fstart == true){ 
							cur->child_head = new; // cur의 자식을 new 로
							new->parent = cur; // new의 부모를 cur로

							fstart = false;
						}
						else{ // 함수가 아니면 형제로 node 연결
							cur->next = new; 
							new->prev = cur;
						}

						cur = new; // 새로 만든 노드로 이동
					}

					if(!strcmp(tokens[*idx], ")")) // 토큰이 닫는 괄호면 탈출
						break;
				}
			}
			else{ // 그 외에는
				*idx += 1; // 다음 토큰으로 이동

				new = make_tree(NULL, tokens, idx, parentheses + 1); // 자식트리 생성

				if(cur == NULL) // 처음이면 new가 cur로
					cur = new;

				else if(!strcmp(new->name, cur->name)){ // cur와 new의 토큰명이 같을때
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||") 
							|| !strcmp(new->name, "&") || !strcmp(new->name, "&&")) // 비교연산자라면
					{
						cur = get_last_child(cur); // 자식중 막내노드로 이동

						if(new->child_head != NULL){ // 자식노드의 자식트리가 존재하면
							new = new->child_head; // 자식노드의 자식으로 이동

							new->parent->child_head = NULL; // 자식관계에서 형제관계로 변경
							new->parent = NULL;
							new->prev = cur;
							cur->next = new;
						}
					}
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*")) // new가 +나 *이면
					{
						i = 0;

						while(1)
						{
							if(!strcmp(tokens[*idx + i], "")) // 토큰이 비었으면 break
								break;

							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0) //연산자이고 닫는괄호가 아니면 break
								break;

							i++;
						}

						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name)) // 새로운 연산자와 기존 연산자의 우선순위를 비교해 새로운 연산자가 우선이면
						{
							cur = get_last_child(cur); //자식중 막내 노드로 이동
							cur->next = new; // 막내 노드를 new로
							new->prev = cur;
							cur = new;
						}
						else // 반대의 경우에는
						{
							cur = get_last_child(cur); // 자식중 막내노드로 이동

							if(new->child_head != NULL){ //새로운 연산자의 자식노드를 기존연산자의 다음 형제로 이동 
								new = new->child_head;

								new->parent->child_head = NULL;
								new->parent = NULL;
								new->prev = cur;
								cur->next = new;
							}
						}
					}
					else{ // 그외
						cur = get_last_child(cur); // 자식 중 막내 노드로 이동
						cur->next = new; // new를 막내로
						new->prev = cur;
						cur = new;
					}
				}

				else // 그 외
				{
					cur = get_last_child(cur); // 막내로 이동

					cur->next = new; // new를 막내로
					new->prev = cur;

					cur = new;
				}
			}
		}
		else if(is_operator(tokens[*idx])) // 토큰이 연산자이면
		{
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&") 
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*"))
			{ // 좌우가 상관없는 연산자이면 
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx])) // cur이 연산자이고 현재 토큰과 일치하면 
					operator = cur; // 연산자로 현재 노드 지정

				else // 일치하지 않으면 
				{
					new = create_node(tokens[*idx], parentheses); // 새로운 노드 생성
					operator = get_most_high_precedence_node(cur, new); // 기존의 우선순위가 가장 높은 연산자와 새로운 연산자의 우선순위 비교 후 연산자로 지정

					if(operator->parent == NULL && operator->prev == NULL){ // 우선순위인 연산자 노드가 혼자이면

						if(get_precedence(operator->name) < get_precedence(new->name)){ // 기존 연산자가 새로운 연산자보다 우선 순위가 높으면
							cur = insert_node(operator, new); // 새로운 연산자를 기존 우선순위 연산자의 부모로 둠 
						}

						else if(get_precedence(operator->name) > get_precedence(new->name)) // 반대의 경우
						{
							if(operator->child_head != NULL){ // 기존 연산자 노드의 자식이 있으면 새로운 연산자 노드를 자식 연산자 노드들의 막내로 바꾸고 막내노드가 새로운 연산자 노드의 자식이 됨
								operator = get_last_child(operator);
								cur = insert_node(operator, new); // op의 자식의 끝에 new 삽입
							}
						}
						else // 같은 경우
						{
							operator = cur; // cur을 연산자로 지정

							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx]))
									break;

								if(operator->prev != NULL) // 기존 연산자의 prev가 있으면
									operator = operator->prev; // 기존 연산자의 prev로 이동 
								else
									break;
							}

							if(strcmp(operator->name, tokens[*idx]) != 0) // 기존 연산자와 새로운 토큰이 다를 경우 
								operator = operator->parent;

							if(operator != NULL){ // 연산자 노드가 있을 경우
								if(!strcmp(operator->name, tokens[*idx])) // 기존 연산자와 새로운 토큰이 같으면 
									cur = operator; // 현재 노드는 연산자 노드로 지정
							}
						}
					}

					else // 자식이나 형제가 있으면
						cur = insert_node(operator, new);
				}

			}
			else // 양 변의 인자 순서가 상관 있는 경우 (부등호, 빼기, 나누기 등)
			{
				new = create_node(tokens[*idx], parentheses); //새로운 노드 생성

				if(cur == NULL) // 노드가 없을 경우 생성
					cur = new;

				else // 노드가 있으면 
				{
					operator = get_most_high_precedence_node(cur, new); // cur와 new의 연산자 우선순위 비교 후 더 높은 노드 할당

					if(operator->parentheses > new->parentheses) // 기존연산자가 새연산자보다 큰 경우
						cur = insert_node(operator, new); 

					else if(operator->parent == NULL && operator->prev ==  NULL){ // 우선 연산자가 최상위 노드이면 

						if(get_precedence(operator->name) > get_precedence(new->name)) // 기존연산자가 새연산자보다 크면
						{
							if(operator->child_head != NULL){ // 자식이 있으면 

								operator = get_last_child(operator); // 자식의 막내의 형 노드로 삽입
								cur = insert_node(operator, new);
							}
						}

						else	
							cur = insert_node(operator, new); // 기존 연산자가 새연산자보다 크면 
					}

					else // 기존 연산자가 새연산자보다 크면
						cur = insert_node(operator, new);
				}
			}
		}
		else  // 토큰이 문자열일 경우
		{
			new = create_node(tokens[*idx], parentheses); // 노드 생성

			if(cur == NULL) // 처음이면 이 노드를 cur로
				cur = new;

			else if(cur->child_head == NULL){ // 자식이 없다면
				cur->child_head = new; // 자식으로 지정
				new->parent = cur;

				cur = new;
			}
			else{ // 자식이 있다면 자식의 막내로 

				cur = get_last_child(cur);

				cur->next = new;
				new->prev = cur;

				cur = new;
			}
		}

		*idx += 1; // 인덱스 증가
	}

	return get_root(cur);
}

node *change_sibling(node *parent) // 형제를(첫째,둘째) 교체해주는 함수 
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

node *create_node(char *name, int parentheses) // 새로운 노드를 만들어주는 함수
{
	node *new;

	new = (node *)malloc(sizeof(node));
	new->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(new->name, name);

	new->parentheses = parentheses;
	new->parent = NULL;
	new->child_head = NULL;
	new->prev = NULL;
	new->next = NULL;

	return new;
}

int get_precedence(char *op) // 우선순위를 구해주는 함수 
{
	int i;

	for(i = 2; i < OPERATOR_CNT; i++){
		if(!strcmp(operators[i].operator, op))
			return operators[i].precedence;
	}
	return false;
}

int is_operator(char *op) // 연산자가 있는지 검사해주는 함수
{
	int i;

	for(i = 0; i < OPERATOR_CNT; i++)
	{
		if(operators[i].operator == NULL)
			break;
		if(!strcmp(operators[i].operator, op)){
			return true;
		}
	}

	return false;
}

node *get_operator(node *cur) // 해당 노드의 부모를 리턴해주는 함수
{
	if(cur == NULL)
		return cur;

	if(cur->prev != NULL)
		while(cur->prev != NULL)
			cur = cur->prev;

	return cur->parent;
}

node *get_root(node *cur) // root를 찾아주는 함수
{
	if(cur == NULL)
		return cur;

	while(cur->prev != NULL)
		cur = cur->prev;

	if(cur->parent != NULL)
		cur = get_root(cur->parent);

	return cur;
}

node *get_high_precedence_node(node *cur, node *new) // 두 트리 중 높은 우선순위를 가진 노드를 구해주는 함수
{
	if(is_operator(cur->name)) // cur노드가 연산자일때
		if(get_precedence(cur->name) < get_precedence(new->name)) // new의 우선순위가 높으면
			return cur; // cur 리턴

	if(cur->prev != NULL){ // cur의 형이 있으면 첫째로 이동
		while(cur->prev != NULL){
			cur = cur->prev;

			return get_high_precedence_node(cur, new); // 첫째와 new를 비교 
		}


		if(cur->parent != NULL) //첫째와 비교 후 새연산자가 더 우선이면
			return get_high_precedence_node(cur->parent, new); // 부모와 비교
	}

	if(cur->parent == NULL) // 부모가 후순위이면 cur 리턴
		return cur;
}

node *get_most_high_precedence_node(node *cur, node *new) // new와 cur
{
	node *operator = get_high_precedence_node(cur, new);
	node *saved_operator = operator;

	while(1)
	{
		if(saved_operator->parent == NULL)
			break;

		if(saved_operator->prev != NULL)
			operator = get_high_precedence_node(saved_operator->prev, new);

		else if(saved_operator->parent != NULL)
			operator = get_high_precedence_node(saved_operator->parent, new);

		saved_operator = operator;
	}

	return saved_operator;
}

node *insert_node(node *old, node *new) // 노드를 삽입해주는 함수
{
	if(old->prev != NULL){ // 형이 있으면
		new->prev = old->prev; // old의 형을 new의 형으로 
		old->prev->next = new; // new를 old로
		old->prev = NULL;
	}

	new->child_head = old; // 새 노드의 자식을 기존 노드로 변경
	old->parent = new; // 기존 노드의 부모를 새 노드로 변경

	return new;
}

node *get_last_child(node *cur) // 마지막자식을 얻어주는 함수
{
	if(cur->child_head != NULL)
		cur = cur->child_head;

	while(cur->next != NULL)
		cur = cur->next;

	return cur;
}

int get_sibling_cnt(node *cur) // 형제의 수를 체크해주는 함수
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

void free_node(node *cur) // 노드의 메모리를 해제해주는 함수
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


int is_character(char c) // 문자인지 리턴해주는 함수 
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_typeStatement(char *str) // 데이터타입인지 확인해주는 함수
{ 
	char *start;
	char str2[BUFLEN] = {0}; 
	char tmp[BUFLEN] = {0}; 
	char tmp2[BUFLEN] = {0}; 
	int i;	 

	start = str;
	strncpy(str2,str,strlen(str));
	remove_space(str2); // str2에 space을 제거

	while(start[0] == ' ')
		start += 1;

	if(strstr(str2, "gcc") != NULL) // "gcc"이 str2에 있으면
	{
		strncpy(tmp2, start, strlen("gcc")); // start의 3바이트를 tmp2에 복사
		if(strcmp(tmp2,"gcc") != 0) // tmp2가 gcc가 아니면
			return 0;
		else // gcc이면
			return 2;
	}

	for(i = 0; i < DATATYPE_SIZE; i++) // datatype인지 검사
	{
		if(strstr(str2,datatype[i]) != NULL) // datatype이 문자열에 존재하는 경우
		{	
			strncpy(tmp, str2, strlen(datatype[i])); 
			strncpy(tmp2, start, strlen(datatype[i])); 

			if(strcmp(tmp, datatype[i]) == 0) // 맨앞이 datatype인 경우
				if(strcmp(tmp, tmp2) != 0)
					return 0;  
				else // 공백없이 잘 입력한 경우
					return 2;
		}

	}
	return 1;

}

int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]) // cast를 확인하고 해당 토큰을 리턴해주는 함수
{
	int i, j;

	for(i = 0; i < TOKEN_CNT; i++)
	{
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			if(strstr(tokens[i], datatype[j]) != NULL && i > 0) // 해당토큰에 데이터타입이 있고 1이상이면
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

int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]) // struct가 있으면 해당토큰 위치를 리턴해주는 함수
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

int all_star(char *str) // 문자열에 *이 있는지 확인 해주는 함수 
{
	int i;
	int length= strlen(str);

	if(length == 0)	
		return 0;

	for(i = 0; i < length; i++)
		if(str[i] != '*')
			return 0;
	return 1;

}

int all_character(char *str) // 문자가 하나라도 있는지 확인해주는 함수
{
	int i;

	for(i = 0; i < strlen(str); i++)
		if(is_character(str[i]))
			return 1;
	return 0;

}

int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]) //
{
	int i;
	int j = start - 1;
	int lcount = 0, rcount = 0;
	int sub_lcount = 0, sub_rcount = 0;

	if(start > -1){ // 해당 토큰의 위치가 0이상일때
		if(!strcmp(tokens[start], "struct")) {	//struct이면	
			strcat(tokens[start], " "); // 공백을 붙이고
			strcat(tokens[start], tokens[start+1]); // 다음 토큰을 붙임 

			for(i = start + 1; i < TOKEN_CNT - 1; i++){ // 남은 토큰 개수 만큼 돌면서
				strcpy(tokens[i], tokens[i + 1]); // 앞으로 한칸씩 밀며 재정렬
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

		else if(!strcmp(tokens[start], "unsigned") && strcmp(tokens[start+1], ")") != 0) {	// unsigned이고 다음 토큰이 닫는 괄호가 아니면
			strcat(tokens[start], " "); // 공백을 붙이고
			strcat(tokens[start], tokens[start + 1]); // 다음 토큰과 
			strcat(tokens[start], tokens[start + 2]); // 다다음 토큰을 붙이고 

			for(i = start + 1; i < TOKEN_CNT - 1; i++){ // 남은 토큰을 재정렬
				strcpy(tokens[i], tokens[i + 1]);
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

		j = start + 1; // 데이터타입 다음 위치를 j에 지정
		while(!strcmp(tokens[j], ")")){ // 닫는괄호 개수 카운트
			rcount ++;
			if(j==TOKEN_CNT)
				break;
			j++;
		}

		j = start - 1; // 데이터 타입 이전 위치를 j에 지정
		while(!strcmp(tokens[j], "(")){ // 여는괄호 개수 카운트
			lcount ++;
			if(j == 0)
				break;
			j--;
		}
		if( (j!=0 && is_character(tokens[j][strlen(tokens[j])-1]) ) || j==0) // 여는 괄호의 좌측이 문자이거나 존재하지 않으면
			lcount = rcount; // 카운트를 통일

		if(lcount != rcount ) // 카운트 다르면 false
			return false;

		if( (start - lcount) >0 && !strcmp(tokens[start - lcount - 1], "sizeof")){ // 데이터타입 위치보다 여는괄호 개수가 많고 여는괄호 좌측이 sizeof이면 true
			return true; 
		}

		else if((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start+1], ")")) { //  데이터타입이 unsigned이거나 struct이고 다음 토큰이 닫는 괄호이면
			strcat(tokens[start - lcount], tokens[start]); // 여는괄호가 있는 토큰에 start를 이어붙임
			strcat(tokens[start - lcount], tokens[start + 1]); // 닫는괄호도 이어붙임
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]); //바로 다음 토큰에 닫는괄호이후 부붙을 이어 붙임

			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount; i++) { // 토큰 재정렬
				strcpy(tokens[i], tokens[i + lcount + rcount]);
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0]));
			}
		}
		else{ // 그 외의 경우
			if(tokens[start + 2][0] == '('){ // 두개 뒤의 토큰이 닫는 괄호이면
				j = start + 2; // 그 위치를 j에 지정
				while(!strcmp(tokens[j], "(")){ // 해당 위치부터 서브열린괄호체크 
					sub_lcount++;
					j++;
				} 	
				if(!strcmp(tokens[j + 1],")")){ // 서브닫힌괄호체크
					j = j + 1;
					while(!strcmp(tokens[j], ")")){
						sub_rcount++;
						j++;
					}
				}
				else // 닫힌괄호 없으면 false
					return false;

				if(sub_lcount != sub_rcount) // 서브괄호의 개수가 맞지 않으면 false
					return false;

				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]); // 서브닫힌괄호 시작부분에 서브 닫힌 괄호 끝부분을 저장
				for(int i = start + 3; i<TOKEN_CNT; i++) // 뒷부분은 제거
					memset(tokens[i], 0, sizeof(tokens[0]));

			}
			strcat(tokens[start - lcount], tokens[start]); // 괄호를 제거하고
			strcat(tokens[start - lcount], tokens[start + 1]);
			strcat(tokens[start - lcount], tokens[start + rcount + 1]);

			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount -1; i++) { //토큰 재정리
				strcpy(tokens[i], tokens[i + lcount + rcount +1]);
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0])); // 그 이후의 토큰 초기화

			}
		}
	}
	return true;
}

void clear_tokens(char tokens[TOKEN_CNT][MINLEN]) // 토큰리스트 비워주는 함수
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i]));
}

char *rtrim(char *_str) // 우측 공백문자를 제거하고 널로 교체
{
	char tmp[BUFLEN];
	char *end;

	strcpy(tmp, _str); 
	end = tmp + strlen(tmp) - 1; // 문자열의 마지막주소
	while(end != _str && isspace(*end)) // 공백문자인 경우 주소 당김
		--end;

	*(end + 1) = '\0'; // 마지막 문자를 널로 지정
	_str = tmp; // 작업 완료된 문자열 저장교환
	return _str; // 작업된 문자열 리턴
}

char *ltrim(char *_str) // 좌측공백문자 제거 함수
{
	char *start = _str;

	while(*start != '\0' && isspace(*start)) // 공백문자인 경우 주소 밈
		++start; 
	_str = start; //작업 완료된 문자열 저장 교환
	return _str;
}

char* remove_extraspace(char *str) // 
{
	int i;
	char *str2 = (char*)malloc(sizeof(char) * BUFLEN);
	char *start, *end;
	char temp[BUFLEN] = "";
	int position;

	if(strstr(str,"include<")!=NULL){ // <를 공백으로 바꿔줌
		start = str;
		end = strpbrk(str, "<");
		position = end - start;

		strncat(temp, str, position);
		strncat(temp, " ", 1);
		strncat(temp, str + position, strlen(str) - position + 1);

		str = temp;		
	}

	for(i = 0; i < strlen(str); i++) // str2에 공백 제거한 결과 저장
	{
		if(str[i] ==' ') // 공백일때 
		{
			if(i == 0 && str[0] ==' ') // 첫 인자가 공백이면
				while(str[i + 1] == ' ') // 공백이 아닐때까지 이동
					i++;	
			else{ // 첫인자가 아닌데 공백이면
				if(i > 0 && str[i - 1] != ' ') // 바로 전 인자가 공백이 아니면
					str2[strlen(str2)] = str[i]; // 공백을 넣고
				while(str[i + 1] == ' ') // 다음 인자가 공백이 아닐때까지 이동
					i++;
			} 
		}
		else // 공백이 아니면 str2에 저장
			str2[strlen(str2)] = str[i];
	}

	return str2; // str2 리턴
}

void remove_space(char *str) // 공백을 제거해주는 함수
{
	char* i = str;
	char* j = str;

	while(*j != 0)
	{
		*i = *j++;
		if(*i != ' ')
			i++;
	}
	*i = 0;
}

int check_brackets(char *str) // 괄호가 올바른지 체크해주는 함수
{
	char *start = str; 
	int lcount = 0, rcount = 0;

	while(1){
		if((start = strpbrk(start, "()")) != NULL){ // "()" 둘 중 하나가 있는 위치로 이동
			if(*(start) == '(') // 좌괄호 체크
				lcount++;
			else // 우괄호 체크
				rcount++;

			start += 1; // 다음 문자위치로 이동		
		}
		else // 없으면 종료
			break;
	}

	if(lcount != rcount) // 괄호 개수가 맞지 않으면 오류
		return 0;
	else 
		return 1;
}

int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]) // 토큰의 개수를 구하는 함수
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		if(!strcmp(tokens[i], ""))
			break;

	return i;
}
