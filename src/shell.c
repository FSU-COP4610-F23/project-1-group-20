#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lexer.c"

void showPrompt();
void print(tokenlist *tokens);
char* replace_environment_variable(char *token);
char* replace_tilda(char *token);

int main()
{
	showPrompt();
	char *input = get_input();
	tokenlist *tokens = get_tokens(input);
	print(tokens);

	// Loop thru tokens...
	for (int i=0; i<tokens->size; i++)
	{
		// Get first character of token.
		switch (tokens->items[i][0])
		{
			case '$' :
				// Replace environment variable.
				tokens->items[i] = replace_environment_variable(tokens->items[i]);
				break;
			case '~' :
				// Replace tilda with home folder.
				tokens->items[i] = replace_tilda(tokens->items[i]);
				break;
		}

	}

	print(tokens);
	free(input);
	free_tokens(tokens);

	//loop thru delimited $PATH
	char * delimitedPATH = strtok(getenv("PATH"),":");
	while (delimitedPATH != NULL)
	{
		/* code */
		printf("%s\n",delimitedPATH);
		delimitedPATH = strtok(NULL, ":");
	}
	
	

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

void print(tokenlist *tokens)
{
	printf("Tokenlist...\n");
	for (int i=0; i<tokens->size; i++)
	{
		printf("%p",&tokens->items[i]);
		printf(" has token ");
		printf("%s\n",tokens->items[i]);
	}
}

char* replace_environment_variable(char* token)
{
	char str[strlen(token)-1];
	for (int i=0; i<strlen(token); i++) 
	{
		str[i] = token[i+1];
	}
	char *b = (char*)malloc(strlen(getenv(str))+1);
	strcpy(b, getenv(str));
	free(token);
	return b;
}

char* replace_tilda(char* token)
{
	char str[strlen(token)-1];
	for (int i=0; i<strlen(token); i++) 
	{
		str[i] = token[i+1];
	}
	char *b = (char*)malloc(strlen(getenv("HOME"))+strlen(token));
	strcpy(b,getenv("HOME"));
	strcat(b,str);
	free(token);
	return b;
}

