#include <stdio.h>  
#include <string.h>  

char * change_word(char *string, char *old_word, char *new_word);  

int main()  
{  
	char string[30];  
	char old_word[10];  
	char new_word[10];  

	printf("Input string: ");  
	fgets(string, sizeof(string), stdin);  

	printf("Input old word: ");  
	scanf("%s", old_word);  

	printf("Input new word: ");  
	scanf("%s", new_word);  

	strcpy(string, change_word(string, old_word, new_word));  
	printf("change string: %s\n", string);  

	return 0;  
}  

char * change_word(char *string, char *old_word, char *new_word)  
{  
	char *token;  
	char temp[100];  

	memset(temp, 0, sizeof(temp));  
	token = strtok(string, "():,");  
	printf("%s\n", token);

	while(token != NULL)  
	{  
		if(0 == strcmp(token, old_word))  
		{  
			strcat(temp, new_word);  
		}  
		else  
		{  
			strcat(temp, token);  
		}  
		strcat(temp, "():, ");  
		token = strtok(NULL, "():,");  
		printf("%s\n", token);
	}  
	temp[strlen(temp)-1] = 0;  

	printf("change string: %s\n", temp);  
} 

