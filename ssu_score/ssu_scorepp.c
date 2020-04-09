#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "blank.h"

extern struct ssu_scoreTable score_table[QNUM]; // 점수테이블 구조체 변수
extern char id_table[SNUM][10]; // 학번 문자열 변수

struct ssu_scoreTable score_table[QNUM];
char id_table[SNUM][10];

char stuDir[BUFLEN]; // 학생 디렉토리명
char ansDir[BUFLEN]; // 정답 디렉토리명
char errorDir[BUFLEN]; /// 에러디렉토리명 
char threadFiles[ARGNUM][FILELEN]; // -t옵션 사용할 문제 파일명
char printId[ARGNUM][FILELEN]; /// -i옵션 사용할 학번

int eOption = false; // 옵션들 on/off
int tOption = false;
int iOption = false;
int mOption = false;

void ssu_score(int argc, char *argv[]) // 체점 시작
{
	printf("1");
	char saved_path[BUFLEN]; // 경로 변수
	int i;

	for(i = 0; i < argc; i++){ // -h 옵션체크 
		if(!strcmp(argv[i], "-h")){
			print_usage();
			return;
		}
	}

	memset(saved_path, 0, BUFLEN); // 경로명을 널로 memset
	if(argc >= 3){ // 가변인자가 3개이상이면
		strcpy(stuDir, argv[1]);
		strcpy(ansDir, argv[2]);
	}

	if(!check_option(argc, argv)) //옵션 체크
		exit(1); // 설정 외의 옵션이 있으면 종료

	getcwd(saved_path, BUFLEN); // 현재 작업 디렉토리 전체경로 저장

	if(chdir(stuDir) < 0){ // 작업디렉토리를 학생디렉토리로 변경
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}
	getcwd(stuDir, BUFLEN); // 학생디렉토리 전체 경로 저장

	chdir(saved_path); // 다시 현재 작업 디렉토리로 이동
	if(chdir(ansDir) < 0){ // 작업디렉토리를 정답 디렉토리로 변경
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN); // 작업디렉토리 전체 경로 저장

	chdir(saved_path); // 다시 현재 작업 디렉토리로 이동

	set_scoreTable(ansDir); // 점수테이블이 있는지 확인하고 세팅
	set_idTable(stuDir); // 학번테이블 생성

	printf("grading student's test papers..\n");
	score_students(); // 체점 시작

	if(iOption != false) // 중복옵션으로 사용될 경우
		do_iOption(argc, argv, iOption);
	return;
}

int check_option(int argc, char *argv[]) // 옵션을 체크하는 함수 
{
	int i, j;
	int c;

	while((c = getopt(argc, argv, "e:thim")) != -1) // 옵션처리
	{
		switch(c){ 
			case 'e': // e옵션
				eOption = true;
				strcpy(errorDir, optarg);

				if(access(errorDir, F_OK) < 0) // 디렉토리 없으면
					mkdir(errorDir, 0755); // 0755로 디렉토리 생성
				else{ // 디렉토리 있으면
					rmdirs(errorDir); // 제거 후 
					mkdir(errorDir, 0755); // 새로 생성
				}
				break;
			case 't': // t옵션
				tOption = true;
				i = optind; // t옵션이 몇 번째 가변인자인지 체크
				j = 0;

				while(i < argc && argv[i][0] != '-'){ // 가변인자 체크

					if(j >= ARGNUM) // 5개 초과 시
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else // lpthread 처리할 배열에 추가
						strcpy(threadFiles[j], argv[i]);
					i++; 
					j++;
				}
				break;
			case 'i': // i옵션
				iOption = optind;
				i = optind; // i옵션이 몇 번째 가변인자인지 체크
				j = 0;

				while(i < argc && argv[i][0] != '-') //가변인자 체크
				{ 
					if(j >= ARGNUM) // 5개 초과 시
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(printId[j], argv[i]);
					i++; 
					j++;
				}
				if(!strcmp(argv[1], "-i")){ // 단독 사용시 
					do_iOption(argc, argv, optind);
					return false;
				}
				break;
			case 'm': // m옵션
				mOption = true;
				break;
			case '?': // 그 외의 옵션 줄 경우
				printf("Unkown option %c\n", optopt);
				return false;
		}
	}

	return true;
}


void do_mOption(char *path) // m옵션을 처리해주는 함수
{
	FILE *fp; 
	int size, offset, now = 0, fd; 
	char mod[FILELEN];
	char qname[FILELEN];
	char score[BUFLEN];
	char *head, *tail;
	double tmp;

	while(1)
	{
		if((fp = fopen(path, "r")) == NULL){ // scoretable.scv를 오픈
			fprintf(stderr, "(do_mOption)file open error for %s\n", path);
			return ;
		}

		fseek(fp, 0, SEEK_END);
		size = ftell(fp); // 파일사이즈 측정
		fseek(fp, now, SEEK_SET); // 마저 탐색하는 곳으로 이동

		printf("Input question's number to modify >> ");
		scanf("%s", mod); // 수정할 문제 번호

		if(!strcmp(mod, "no")) 
			break;
		else
			while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF)
			{
				strcpy(qname, strtok(qname, "."));

				if(!strcmp(qname, mod)){ // 맞는 문제번호를 찾으면
					printf("Current score : %s\n", score);
					printf("New score : ");
					scanf("%s", mod); // 새로운 점수를 입력함
					tmp = atof(mod);
					sprintf(mod,"%.2f\n", tmp);

					fseek(fp, 0, SEEK_CUR);
					now = ftell(fp); // 마지막 탐색 지점 저장
					tail = (char*)malloc(sizeof(char) * (size - now)); // 탐색 지점 기준 뒤쪽
					fread(tail, size-now, 1, fp);

					fseek(fp, -(strlen(score) + 1 + size - now), SEEK_CUR); // 바뀔 점수 시작 지점
					offset = ftell(fp);
					head = (char*)malloc(sizeof(char) * offset); //시작 지점 앞쪽
					fseek(fp, 0, SEEK_SET);
					fread(head, offset, 1, fp);

					if((fd = open(path, O_WRONLY | O_TRUNC)) < 0){ // 파일을 재생성해줌
						fprintf(stderr, "file open error for %s\n", path);
						return ;
					}

					write(fd, head, offset); // 순서대로 다시 write
					write(fd, mod, strlen(mod));
					write(fd, tail, size - now);
					close(fd);
					fclose(fp);
					free(head);
					free(tail);
				}
			}
	}
	fclose(fp);
}

void do_iOption(int argc, char* argv[], int optind) // i옵션을 처리하는 함수
{
	FILE *fp;
	int i, j, k, qnum = 0;
	double d_score;
	char *id, *score;
	char *qname;
	char tmp[BUFLEN];
	char qlist[QNUM][FILELEN];
	int size = sizeof(printId) / sizeof(printId[0]);

	if((fp = fopen("score.csv", "r")) == NULL){
		fprintf(stderr, "file open error for score.csv\n");
		return ;
	}

	fscanf(fp, "%s\n", tmp);
	qname = strtok(tmp, ","); // 문제파일명

	while(strcmp(qname, "sum")) // 문제들을 리스트에 저장
	{
		strcpy(qlist[qnum++] , qname);
		qname = strtok(NULL, ",");
	} 

	for(i = 0; i < size; i++) // 확일해야할 학번 탐색
	{
		while(fscanf(fp, "%s\n", tmp) != EOF)
		{
			id = strtok(tmp, ","); // score.csv에서 일치하는 학번 탐색
			if(!strcmp(id, printId[i])){
				printf("%s's wrong answer :\n", printId[i]);
				k = qnum;
				j = 0;
				score = strtok(NULL, ","); 
				while(k--) // 틀린문제 탐색
				{
					d_score = atof(score);
					if(d_score == 0)
						printf("%s ", qlist[j]); // 틀린 문제 출력
					score = strtok(NULL, ",");
					j++; 
				} 
				printf("\n");
				break;
			}
		}
		fseek(fp, 0, SEEK_SET);
		fscanf(fp, "%s\n", tmp);
	} 
	fclose(fp);
}

void set_scoreTable(char *ansDir) // 점수테이블을 세팅하는 함수
{
	char *filename = "score_table.csv"; // 현재 디렉토리에 score_table.csv 형태

	if(access(filename, F_OK) == 0)  // 기존 파일 있으면
		read_scoreTable(filename); // 점수 파일 읽음
	else{ //없으면
		make_scoreTable(ansDir); //만들고
		write_scoreTable(filename); //씀
	}

	if(mOption) // m옵션 있을시 점수 수정
		do_mOption(filename);
}

void read_scoreTable(char *path)
{
	FILE *fp; // 파일 포인터
	char qname[FILELEN]; //문제명
	char score[BUFLEN]; //점수
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){ // 읽기전용으로 점수테이블 open
		fprintf(stderr, "(read_scoreTable)file open error for %s\n", path);
		return ;
	}

	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){ // ,가 나올때까지 / 그리고 끝까지 읽음
		strcpy(score_table[idx].qname, qname); // 문제명과
		score_table[idx++].score = atof(score); //점수를 저장
	}

	fclose(fp);
}

void make_scoreTable(char *ansDir)
{
	int type, num;
	double score, bscore, pscore;
	struct dirent *dirp, *c_dirp;
	DIR *dp, *c_dp;
	char tmp[BUFLEN];
	int idx = 0;
	int i;

	num = get_create_type(); // 생성방식을 설정

	if(num == 1) // 1번(일괄방식)
	{
		printf("Input value of blank question : ");
		scanf("%lf", &bscore); //빈칸 문제 점수
		printf("Input value of program question : ");
		scanf("%lf", &pscore); // 프로그램 문제 점수
	}

	if((dp = opendir(ansDir)) == NULL){ // 점수 디렉토리 오픈
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}	

	while((dirp = readdir(dp)) != NULL) // dirent 획득
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) //경로 시작이 부모나 자신이면
			continue;

		if((type = get_file_type(dirp->d_name)) < 0)
		strcpy(score_table[idx++].qname, dirp->d_name);
	}

	closedir(dp);
	sort_scoreTable(idx); // 문제 번호를 오름차순으로 정리

	for(i = 0; i < idx; i++)
	{
		type = get_file_type(score_table[i].qname); // 문제의 파일타입을 받아냄

		if(num == 1) // 1번 케이스
		{
			if(type == TEXTFILE)
				score = bscore;
			else if(type == CFILE)
				score = pscore;
		}
		else if(num == 2) // 2번케이스
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score);
		}

		score_table[i].score = score;
	}
}

void write_scoreTable(char *filename) // 스코어테이블에 입력하는 함수
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]);

	if((fd = creat(filename, 0666)) < 0){
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++)
	{
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score);
		write(fd, tmp, strlen(tmp));
	}

	close(fd);
}

void set_idTable(char *stuDir) // 학번테이블을 만들어주는 함수
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

	if((dp = opendir(stuDir)) == NULL){
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}

	while((dirp = readdir(dp)) != NULL){
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name);
		stat(tmp, &statbuf);

		if(S_ISDIR(statbuf.st_mode))
			strcpy(id_table[num++], dirp->d_name);
		else
			continue;
	}

	sort_idTable(num);
}

void sort_idTable(int size) // 학번을 오름차순으로 정렬해주는 함수
{
	int i, j;
	char tmp[10];

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 -i; j++){
			if(strcmp(id_table[j], id_table[j+1]) > 0){
				strcpy(tmp, id_table[j]);
				strcpy(id_table[j], id_table[j+1]);
				strcpy(id_table[j+1], tmp);
			}
		}
	}
}

void sort_scoreTable(int size) // 문제순서에 맞게 조정
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 - i; j++){

			get_qname_number(score_table[j].qname, &num1_1, &num1_2);
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2);


			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){

				memcpy(&tmp, &score_table[j], sizeof(score_table[0]));
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			}
		}
	}
}

void get_qname_number(char *qname, int *num1, int *num2)
{
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname));
	*num1 = atoi(strtok(dup, "-."));

	p = strtok(NULL, "-.");
	if(p == NULL)
		*num2 = 0;
	else
		*num2 = atoi(p);
}

int get_create_type() // 문제 점수 테이블 설정 방식 선택 함수
{
	int num;

	while(1)
	{
		printf("score_table.csv file doesn't exist in TREUDIR!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num);

		if(num != 1 && num != 2)
			printf("not correct number!\n");
		else
			break;
	}

	return num;
}

void score_students() //체점을 시작하는 함수
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]);

	if((fd = creat("score.csv", 0666)) < 0){
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	write_first_row(fd); // 첫 줄 입력

	for(num = 0; num < size; num++)
	{
		if(!strcmp(id_table[num], ""))
			break;

		sprintf(tmp, "%s,", id_table[num]);

		write(fd, tmp, strlen(tmp)); 

		score += score_student(fd, id_table[num]); //
	}

	printf("Total average : %.2f\n", score / num);

	close(fd);
}

double score_student(int fd, char *id) // 총점을 계산해주는 함수
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);

	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname);

		if(access(tmp, F_OK) < 0)
			result = false;
		else
		{
			if((type = get_file_type(score_table[i].qname)) < 0)
				continue;

			if(type == TEXTFILE)
				result = score_blank(id, score_table[i].qname); //
			if(type == CFILE)
				result = score_program(id, score_table[i].qname); //
		}

		if(result == false) // 틀릴경우
			write(fd, "0,", 2);
		else{
			if(result == true){ // 완벽히 맞은 경우
				score += score_table[i].score;
				sprintf(tmp, "%.2f,", score_table[i].score);
			}
			else if(result < 0){ // 부분점수의 경우
				score = score + score_table[i].score + result;
				sprintf(tmp, "%.2f,", score_table[i].score + result);
			}
			write(fd, tmp, strlen(tmp)); // 해당 문제의 점수를 score.csv에 저장
		}
	}

	printf("%s is finished. score : %.2f\n", id, score); 

	sprintf(tmp, "%.2f\n", score);
	write(fd, tmp, strlen(tmp)); // 해당 학생 총점

	return score;
}

void write_first_row(int fd) // score.csv의 첫 줄 입력 함수
{
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);

	write(fd, ",", 1); // ,를 먼저 입력

	for(i = 0; i < size; i++){
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s,", score_table[i].qname);
		write(fd, tmp, strlen(tmp));
	}
	write(fd, "sum\n", 4);
}

char *get_answer(int fd, char *result) // ':'로 구분된 정답을 분리해주는 함수
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN);
	while(read(fd, &c, 1) > 0)
	{
		if(c == ':') // ':'이 나오면 break
			break;

		result[idx++] = c;
	}
	if(result[strlen(result) - 1] == '\n') // 유일한 답이 존재하는 경우
		result[strlen(result) - 1] = '\0';

	return result;
}

int score_blank(char *id, char *filename) // 빈칸문제 점수처리
{
	char tokens[TOKEN_CNT][MINLEN];
	node *std_root = NULL, *ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN];
	char qname[FILELEN];
	int fd_std, fd_ans;
	int result = true;
	int has_semicolon = false;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); // 확장자 제외한 문제번호

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename);
	fd_std = open(tmp, O_RDONLY);
	strcpy(s_answer, get_answer(fd_std, s_answer)); // 해당 문제의 학생 답을 얻어옴

	if(!strcmp(s_answer, "")){ // 답을 기입못한 경우
		close(fd_std);
		return false;
	}

	if(!check_brackets(s_answer)){ // 괄호의 개수가 맞지 않는 경우
		close(fd_std);
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer))); // 앞과 뒤의 공백문자 제거 후 복사

	if(s_answer[strlen(s_answer) - 1] == ';'){ // 세미콜론이 있는지 체크
		has_semicolon = true; // 있으면
		s_answer[strlen(s_answer) - 1] = '\0'; // 널문자로 바꿔줌
	}

	if(!make_tokens(s_answer, tokens)){ // 토큰으로 분리 
		close(fd_std);
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0); // 학생 답 토큰들을 트리화

	sprintf(tmp, "%s/%s", ansDir, filename);
	fd_ans = open(tmp, O_RDONLY);

	while(1)
	{
		ans_root = NULL;
		result = true;

		for(idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx]));

		strcpy(a_answer, get_answer(fd_ans, a_answer)); // ':'로 구분된 정답 분리

		if(!strcmp(a_answer, ""))
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer))); // 앞과 뒤의 공백문자 제거

		/** 세미콜론 처리 부분 **/
		if(has_semicolon == false){ 
			if(a_answer[strlen(a_answer) -1] == ';')
				continue;
		}

		else if(has_semicolon == true)
		{
			if(a_answer[strlen(a_answer) - 1] != ';')
				continue;
			else
				a_answer[strlen(a_answer) - 1] = '\0';
		}

		if(!make_tokens(a_answer, tokens))
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0);

		compare_tree(std_root, ans_root, &result);

		if(result == true){
			close(fd_std);
			close(fd_ans);

			if(std_root != NULL)
				free_node(std_root);
			if(ans_root != NULL)
				free_node(ans_root);
			return true;

		}
	}

	close(fd_std);
	close(fd_ans);

	if(std_root != NULL) // 트리 메모리 해제
		free_node(std_root);
	if(ans_root != NULL)
		free_node(ans_root);

	return false;
}

double score_program(char *id, char *filename) // 프로그램 문제 점수를 구해주는 함수
{
	double compile;
	int result;

	compile = compile_program(id, filename); // 컴파일오류와 감점사항을 리턴

	if(compile == ERROR || compile == false)
		return false;

	result = execute_program(id, filename); // 프로그램을 실행해서 비교하여 정답여부 리턴

	if(!result) // 일치하지 않으면
		return false;

	if(compile < 0) // 감점 사항이 있으면
		return compile;

	return true; // 정답이면
}

int is_thread(char *qname) // 쓰레드 파일인지 검사해주는 함수
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]);

	for(i = 0; i < size; i++){
		if(!strcmp(threadFiles[i], qname))
			return true;
	}
	return false;
}

double compile_program(char *id, char *filename) // 컴파일오류와 감점사항을 리턴해주는 함수
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN];
	int isthread;
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); // 파일명에서 .빼고 qname에 복사

	isthread = is_thread(qname); // 쓰레드 파일인지 검사

	sprintf(tmp_f, "%s/%s", ansDir, filename); // .c 파일저장형태
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname); // .exe 파일저장형태

	if(tOption && isthread) // t 옵션일시
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else // 그 외
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_e, "%s/%s_error.txt", ansDir, qname); // 에러메시지(정답파일)출력 파일경로
	fd = creat(tmp_e, 0666);

	redirection(command, fd, STDERR); // 
	size = lseek(fd, 0, SEEK_END);
	close(fd);
	unlink(tmp_e); //에러메시지(정답파일) 삭제

	if(size > 0) //에러 출력시  정답파일 잘못됨
		return false;
	

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename); // 학생 실행파일생성
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname);

	if(tOption && isthread)
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname);
	fd = creat(tmp_f, 0666);

	redirection(command, fd, STDERR);
	size = lseek(fd, 0, SEEK_END);
	close(fd);

	if(size > 0){ // 학생답안파일에 컴파일에러가 있는 경우
		if(eOption)
		{
			sprintf(tmp_e, "%s/%s", errorDir, id);
			if(access(tmp_e, F_OK) < 0)
				mkdir(tmp_e, 0755);

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname);
			rename(tmp_f, tmp_e); // stuDir >> errorDir

			result = check_error_warning(tmp_e); // 감점체크
		}
		else{ 
			result = check_error_warning(tmp_f);
			unlink(tmp_f); // 학생디렉토리에 에러출력 텍스트파일 언링크
		}

		return result;
	}

	unlink(tmp_f); // 학생디렉토리에 에러출력 텍스트파일 언링크
	return true;
}

double check_error_warning(char *filename) // error, warning 감지해 감점처리해주는 함수
{
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;

	if((fp = fopen(filename, "r")) == NULL){
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	while(fscanf(fp, "%s", tmp) > 0){
		if(!strcmp(tmp, "error:"))
			return ERROR;
		else if(!strcmp(tmp, "warning:"))
			warning += WARNING;
	}

	return warning;
}

int execute_program(char *id, char *filename) // 파일들을 실행해 비교하는 함수
{
	char std_fname[BUFLEN], ans_fname[BUFLEN];
	char tmp[BUFLEN];
	char qname[FILELEN];
	time_t start, end;
	pid_t pid;
	int fd;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));

	sprintf(ans_fname, "%s/%s.stdout", ansDir, qname);
	fd = creat(ans_fname, 0666);

	sprintf(tmp, "%s/%s.exe", ansDir, qname);
	redirection(tmp, fd, STDOUT); // exe파일을 실행해 stdout에 저장
	close(fd);

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname);
	fd = creat(std_fname, 0666);

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname); // 백그라운드로 stdexe파일 실행

	start = time(NULL);
	redirection(tmp, fd, STDOUT);

	sprintf(tmp, "%s.stdexe", qname);
	while((pid = inBackground(tmp)) > 0){ // 프로세스가 실행 중이면
		end = time(NULL); // 끝나는 시간 재설정

		if(difftime(end, start) > OVER){ // 설정해놓은 시간 초과 시
			kill(pid, SIGKILL); // 해당 pid 프로세스 종료 
			close(fd);
			return false;
		}
	} 

	close(fd);

	return compare_resultfile(std_fname, ans_fname); // 학생과 정답 파일 비교 후 리턴
}

pid_t inBackground(char *name) // pid값 리턴해주는 함수
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;

	memset(tmp, 0, sizeof(tmp));
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); // background파일을 생성

	sprintf(command, "ps | grep %s", name); // 해당파일의 프로세스 부분검색
	redirection(command, fd, STDOUT);

	lseek(fd, 0, SEEK_SET);
	read(fd, tmp, sizeof(tmp));

	if(!strcmp(tmp, "")){ // 프로세스 검색 결과가 없으면
		unlink("background.txt"); //백그라운드 파일 언링크 
		close(fd);
		return 0;
	}

	pid = atoi(strtok(tmp, " ")); // pid값 추출
	close(fd);

	unlink("background.txt");
	return pid;
}

int compare_resultfile(char *file1, char *file2) // 컴파일 결과물 비교하는 함수
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY);
	fd2 = open(file2, O_RDONLY);

	while(1)
	{
		while((len1 = read(fd1, &c1, 1)) > 0){
			if(c1 == ' ') 
				continue;
			else 
				break;
		}
		while((len2 = read(fd2, &c2, 1)) > 0){
			if(c2 == ' ') 
				continue;
			else 
				break;
		}

		if(len1 == 0 && len2 == 0)
			break;

		to_lower_case(&c1);
		to_lower_case(&c2);

		if(c1 != c2){
			close(fd1);
			close(fd2);
			return false;
		}
	}
	close(fd1);
	close(fd2);
	return true;
}

void redirection(char *command, int new, int old) // 디스크립터를 변환해서 명령하는 함수
{
	int saved;

	saved = dup(old);
	dup2(new, old);

	system(command);

	dup2(saved, old);
	close(saved);
}

int get_file_type(char *filename) // 파일의 타입을 확인하는 함수
{
	char *extension = strrchr(filename, '.');

	if(!strcmp(extension, ".txt"))
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))
		return CFILE;
	else
		return -1;
}

void rmdirs(const char *path) //디렉토리를 삭제해주는 함수
{
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[BUFLEN];

	if((dp = opendir(path)) == NULL)
		return;

	while((dirp = readdir(dp)) != NULL)
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", path, dirp->d_name);

		if(lstat(tmp, &statbuf) == -1)
			continue;

		if(S_ISDIR(statbuf.st_mode))
			rmdirs(tmp);
		else
			unlink(tmp);
	}

	closedir(dp);
	rmdir(path);
}

void to_lower_case(char *c) // 대문자를 소문자로 치환하는 함수
{
	if(*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage() // usage출력 함수
{
	printf("Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -m                modify question's score\n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file \n");
	printf(" -t <QNAMES>       compile QNAME.c with -lpthread option\n");
	printf(" -i <IDS>          print ID's wrong qestions\n");
	printf(" -h                print usage\n");
}
