#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lexer.c"

void showPrompt();
void getUserInput();

int main() {

	showPrompt();
	
	char *input = get_input();

	tokenlist *tokens = get_tokens(input);
	for (int i=0; i<tokens->size; i++)
	{
		char firstChar = tokens->items[i][0];
		int tokenLength = strlen(tokens->items[i]);
		bool foundDollar = false;
		bool foundTilde = false;

		switch (firstChar)
		{
			case '$' :
				foundDollar = true;
				break;
			case '~' :
				foundTilde = true;
				break;
		}

		if (foundDollar) 
		{
				char envvar[tokenLength-1];
				for (int j=0; j<tokenLength; j++)
				{
					envvar[j] = tokens->items[i][j+1];
				}
				char *str = getenv(envvar);
				//char *str1 = (char *) malloc(getenv(envvar)+1);
				char *str2 = (char *) malloc(strlen(str)+1);
				strcpy(str2, str); 
				free(tokens->items[i]);
				tokens->items[i] = str2; 
				printf("%s\n", str);
				printf("%s\n", str2);
				printf("%s\n", tokens->items[i]);
				
				//printf("%p\n", &str);
				//printf("%p\n", &str2);
		}
	}

	

	free(input);
	free_tokens(tokens);


	return 0;
}

void showPrompt()
{
	char *str1 = getenv("USER");
	char *str2 = getenv("MACHINE");
	char *str3 = getenv("PWD");

	printf("%s@", str1);
	printf("%s:", str2);
	printf("%s>", str3);
}

/*
void getUserInput()
{
	char userInput[30];

	fgets(userInput, sizeof(userInput), stdin);

	printf("You typed %s", userInput);
}
*/

