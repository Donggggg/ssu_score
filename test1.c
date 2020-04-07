#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define MAX_WORD 100
#define MAX_ANSWER 5

typedef struct parse_tree{
	char word[BUFFER_SIZE];
	bool bracket;
	struct parse_tree* right;
	struct parse_tree* left;
}parse_node;

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

parse_node* DivideSentence(char* sen) //
{
	int i, pos, save = 0, count = 0;
	char word_list[30][MAX_WORD];
	char *start, *end;
	char *op = "(),<=!|&^/+-*\"";
	sen = strdup(sen);

	while (strpbrk(sen, ">") != NULL)
		ChangeInequ(sen);
	printf("%s\n", sen);

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
			if (!IsExist(sen[pos-1], op))
				strncpy(word_list[count++], sen+save, pos - save);
			save = ++pos;
		}
		else { // 문자일 때
			if (pos == strlen(sen) - 1) { }
			else
				pos++;
		}
	} 

	for(i = 0; i < count; i++)
		printf("%s\n", word_list[i]);
}

int main(void)
{
	int i, j, k, count = 0;
	char ans[BUFFER_SIZE] = "lseek(fd, 15000, SEEK_SET):lseek(fd, 15000, 0)";
	char std[BUFFER_SIZE] = "lseek(fd, 15000, 0)";
	char answer_case[MAX_ANSWER][BUFFER_SIZE / MAX_ANSWER];
	char* string;

	char* test = "(value>=STOP1)||(value>=STOP2)";
	char* test1 = "-1==(fd1==open(filename,O_RDWR|O_APPEND,0644))";
//	test =	ChangeInequ(test);

	strcpy(ans, EraseSpace(ans));

	parse_node *head = malloc(sizeof(parse_node));
	parse_node *now;

	/** 구분문자 ':' 기준으로 정답SET 분리 **/
	string = strtok(ans, ":");
	while (string != NULL) {
		strcpy(answer_case[count++], string);
		string = strtok(NULL, ":");
	}

	/** 정답 케이스 순회 **/
//	for(i = 0; i < count; i++) {
			//	DivideSentence(answer_case[i]);
				DivideSentence(test1);
//	}

}
