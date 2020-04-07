#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define MAX_WORD 100
#define MAX_ANSWER 5

typedef struct parse_tree{
	char word[BUFFER_SIZE];
	bool bracket;
	int depth;
	struct parse_tree* right;
	struct parse_tree* left;
}parse_node;

int CountWords(char** wordlist);

char* EraseSpace(char* origin) // 공백제거 함수
{
	int i, index = 0;
	char* string = malloc(sizeof(char) * BUFFER_SIZE);

	for (i = 0; origin[i] != '\0'; i++)
		if (origin[i] != ' ')
			string[index++] = origin[i];

	return string;
}

int WhereExist(char* dest, char* text)
{
	int i, j;

	for (i = 0; i < strlen(dest); i++)
		for(j = 0; j < strlen(text); j++)
			if (text[j] == dest[i])
				return i; // 찾으면 해당 인덱스 리턴

	return i; // 없으면 마지막 인덱스 리턴
}

bool IsExist(char dest, char* text)
{
	int i;

	for (i = 0; i < strlen(text); i++)
		if (text[i] == dest)
			return true;
	return false;
}

char* ChangeInequ(char* sentence) // 부등호 방향 변경 함수
{
	printf("change inequialitu\n");
	sentence = strdup(sentence);
	int i, j, pos_inequ, pos_op, index = 0, check = 1;
	char *fore, *back, *temp;
	char *op = ",<=!|&^/+-*\"";
	char *op2 = "(),<=!|&^/+-*\"";
	fore = malloc(sizeof(char) * MAX_WORD);
	back = malloc(sizeof(char) * MAX_WORD);
	temp = malloc(sizeof(char) * 2 * MAX_WORD);

	for (i = 0; i < strlen(sentence); i++){
		if (sentence[i] == '>') {
			sentence[i] = '<';
			pos_inequ = i;
			if (sentence[i+1] == '='){
				check = 2;
				pos_inequ = ++i;
			}
			i++;
			break;
		}
	}

	while (1) { // 부등호 뒤 
		if (sentence[i] == '(') { // 괄호 있는 경우
			back[index++] = '(';
			i++;
			while (sentence[i] != ')')
				back[index++] = sentence[i++]; 
			back[index] = ')';
			break;
		}
		else { // 괄호 없는 경우 
			pos_op = WhereExist(sentence+i, op2); 
			strncpy(back, sentence+i, pos_op);
			break;
		}
	} 

	index = 0;
	i = pos_inequ - check;

	while (1) { // 부등호 앞
		if (sentence[i] == ')'){
			while (1) {
				if (!(IsExist(sentence[i], op))){
					i--;
					index++;
					if (sentence[i] == '('){
						index++;
						break;
					}
				}
			}
			break;
		}
		else if (i == 0){
			if (sentence[i] == '('){
				index--;
				i++;
			}
			index++;
			break;
		}
		else if (!(IsExist(sentence[i], op2))){
			i--;
			index++;
		}
		else {
			i++;
			break;
		}
	}

	strncpy(fore, sentence + i, index);

	strcpy(temp, back);
	if (check == 1) {
		strcpy(temp + strlen(back), "<");
		strcpy(temp + strlen(back) + 1, fore);
	}
	else {
		strcpy(temp + strlen(back), "<=");
		strcpy(temp + strlen(back) + 2, fore);
	}

	for(j = 0; j < strlen(temp); j++)
		sentence[i++] = temp[j];

	return sentence;
}

char** DivideSentence(char* sen) // 문자열 리스트를 만들어주는 함수
{
	int i, pos=0, save = 0, count = 0;
	char** word_list = malloc(sizeof(char*) * 30);
	char *start, *end;
	char *op = "(),<=!|&^/+-*\"";
	sen = strdup(sen);

	while (strpbrk(sen, ">") != NULL)
		sen = ChangeInequ(sen);

	while (1) {
		if (pos == strlen(sen))
			break;

		if (sen[pos] == '-' || sen[pos] == '&') {
			if (pos == 0) 
				pos++;
			else if (IsExist(sen[pos-1], op))
				pos++;
		}
	
		if (IsExist(sen[pos], op)) { // 연산자일 때
			if (pos == 0){}
			else if (!IsExist(sen[pos-1], op)){
				word_list[count] = malloc(sizeof(char) * 100);
				strncpy(word_list[count++], sen+save, pos - save);
			}
			save = ++pos;
		}
		else { // 문자일 때
			if (pos == strlen(sen) - 1) { 
				word_list[count] = malloc(sizeof(char) * 100);
				strncpy(word_list[count++], sen+save, pos - save);
				pos++;
			}
			else
				pos++;
		} 
	}  

	for(i=0;i<CountWords(word_list);i++)
		printf("%s\n", word_list[i]);
	return word_list;
}

int CountWords(char** wordlist)
{
	int i = 0;

	while (1) {
		if(wordlist[i] == NULL)
			break;
		else
			i++;
	}
	return i;
}

char* TranslateToNum(char** wordlist, char* sen) 
{
	int i, j, count = 0;
	char* trans_sen = malloc(sizeof(char) * BUFFER_SIZE);
	char *token, *re_op, *str_num; 
	char *op = "(),<=!|&^/+-*\"";

	re_op = strpbrk(sen, op);
	token = strtok(sen, re_op);

	for (i = 0; i < CountWords((wordlist)); i++) {
		sprintf(str_num, "%d", i);
		while (token != NULL) {
			if (0 == strcmp(token,wordlist[i]))
				strcat(trans_sen, str_num);
			else
				strcat(trans_sen, token);
			strcat(trans_sen, re_op);
			re_op = strpbrk(sen, op);
			token = strtok(sen, re_op);

		}
	}
	printf("%s\n", trans_sen);

}

parse_node * MakeTree(char** wordlist, char* sen) // 파스트리 생성
{
	parse_node* head = malloc(sizeof(parse_node));


}

int main(void)
{
	int i, j, k, count = 0;
	char ans[BUFFER_SIZE] = "lseek(fd, 15000, SEEK_SET):lseek(fd, 15000, 0)";
	char std[BUFFER_SIZE] = "lseek(fd, 15000, 0)";
	char answer_case[MAX_ANSWER][BUFFER_SIZE / MAX_ANSWER];
	char** wordlist = malloc(sizeof(char*) * 30);
	char* string;

	char* test = "(value>=STOP1)||(value>=STOP2)";
	char* test1 = "-1==(fd1==open(filename,O_RDWR|O_APPEND,0644))";
	char* test2 = "lseek(&fd,15000,0)";

	strcpy(ans, EraseSpace(ans));

	printf("main\n");
	/** 구분문자 ':' 기준으로 정답SET 분리 **/
	string = strtok(ans, ":");
	while (string != NULL) {
		strcpy(answer_case[count++], string);
		string = strtok(NULL, ":");
	}

	/** 정답 케이스 순회 **/
	//	for(i = 0; i < count; i++) {
	//	DivideSentence(answer_case[i]);
	wordlist =	DivideSentence(test2);
	TranslateToNum(wordlist, test2);
	//	}

}
