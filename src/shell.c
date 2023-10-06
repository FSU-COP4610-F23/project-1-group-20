#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "lexer.h"

void showPrompt();
void print(tokenlist *tokens);
char* replace_environment_variable(char *token);
char* replace_tilda(char *token);
char* get_path_to_command(char* command);
tokenlist* convert_tokens(tokenlist* tokens);
void inputRedirection(tokenlist* tokens, int fileIndex);
void outputRedirection(tokenlist* tokens, int fileIndex);

struct commandTable
{
	tokenlist* items[10];
	int size;
};

//////////////////////////////// MAIN //////////////////////////////
int main()
{
	int fileIndex = 0; 	//Keeps track of index of file
	char *input = "start";	// Holds user input typed into shell.
	tokenlist *tokens;	// User input is broken into tokens.
	int outfd;
	int fd;
	int infd;
	int inputRedirect = 0; // check if we use input redirction < 
	int outputRedirect = 0; // check if we use output redirection >
	int processCounter = 0; 
	int counter = 0; 
	struct commandTable ct; ct.size=0; 

	// START
	while (strcmp(input, "exit")!= 0)
	{
		showPrompt();
		input = get_input(); 
		tokens = get_tokens(input);
		print(tokens);
		tokens = convert_tokens(tokens);

		//trying to use the ct to tokenize items before goes into the for loop below
		/*for (int i=0; i<tokens->size; i++) 
		{
			switch (tokens->items[i][0])
			{
				case '>':
					processCounter++; 
					while(tokens->items[counter][0] != '>')
					{
						ct.items[0][counter] = get_tokens(tokens->items[counter]);
						counter++;
					}
					break;
				case '<':
					processCounter++; 
					break;
				case '|':
					processCounter++; 
					break;
			}
		}

		if (processCounter > 1)
		{
			//
		}*/


		// Loop thru tokens...
		for (int i=0; i<tokens->size; i++)
		{
			// Get first character of token...
			switch (tokens->items[i][0])
			{
				case '|' :
					// If we reach a "pipe" then the next token should be a command token.
					processCounter++;
					break;
				case '>' :
					//output redirection
					fileIndex = i +1;
					outputRedirect = 1 ; //used in child process to run output redirection 
					processCounter++;
					break;
				case '<' :
					//input redirection
					fileIndex = i + 1; 
					inputRedirect = 1; 
					processCounter++; 
					break;
			}
		}
		printf("%s\n", "Printing ct.items[0]");
		ct.items[0] = tokens;
		print(ct.items[0]); 


		// Make Child.
		__pid_t pid = fork();
		int status; 
		if (pid == 0) 
		{
			/*if(processCounter > 1)
			{
				tokens->items[0] = multipleProcesses(); 
			}*/

			if(inputRedirect ==1)
			{
				inputRedirect = 0 ;
				tokens->items[fileIndex-1] = NULL;
				inputRedirection(tokens, fileIndex);
			}

			if (outputRedirect==1)
			{
				outputRedirect = 0;
				tokens->items[fileIndex-1] = NULL;
				outputRedirection(tokens,fileIndex);
			}

			// Execute command.
			execv(tokens->items[0], tokens->items);
		}
		else if(pid < 0)
		{
			fprintf(stderr,"Fork failed");
			return 1; 
		}
		else
		{
			waitpid(pid, &status, 0);
			
			printf("Child Complete\n");
				//exit(0); 
		}


		free(input);
		free_tokens(tokens);
	}
	return 0;
}
//////////////////////////////////////// FUNCTIONS //////////////////////////////////////
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
	for (int i=0; i<tokens->size; i++) {printf("%s\n",tokens->items[i]);}
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

// First we get the paths from $PATH environment variable, tokenize this
// path list and append the command name to end of each path string. Then we 
// test each path with the access() function to see if that path is accessible.
// If so, this function will return the full path to where the command is located.
// If no path found, function returns NULL.
char* get_path_to_command(char* command)
{	
	char *b;
	char *paths;
	paths = (char*)malloc(strlen(getenv("PATH")+1));
	strcpy(paths,getenv("PATH"));
	char *delimitedPATH = strtok(paths,":");
	while (delimitedPATH != NULL)
	{	
		b = (char*)malloc(100);

		// Append the command we are searching for to end of path.
		strcpy(b,delimitedPATH);
		strcat(b,"/");
		strcat(b,command);

		// If path is accessible, return it.
		if (access(b, F_OK) == 0) {return b;}

		// Otherwise, move to next path.
		delimitedPATH = strtok(NULL,":");
		b = NULL;
	}
	// If we've gotten here, command was not found at any path. Return NULL.
	return b;
}


// Performs initial token conversions.
// Expands ~
// Converts $USER, $MACHINE, $PATH etc.
// Converts command names. For example, "ls" becomes "/bin/ls"
tokenlist* convert_tokens(tokenlist* tokens)
{
	// Loop thru tokens...
	for (int i=0; i<tokens->size; i++)
	{
		// First token should be a command like "ls" "echo" etc.
		if (i == 0)
		{
			// Do path search to locate it on the OS.
			tokens->items[0] = get_path_to_command(tokens->items[i]);

			if(tokens->items[0]==NULL)
			{
				printf("command not found.\n");
				exit(0);
			}
		}

		// Get first character of token...
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
			case '|' :
				// If we reach a "pipe" then the next token should be a command token.
				tokens->items[i+1] = get_path_to_command(tokens->items[i+1]);
				break;
		}
	}
	return tokens;
}

void inputRedirection(tokenlist* tokens, int fileIndex)
{
	int fd = open(tokens->items[fileIndex], O_RDONLY);
	int infd = dup(STDIN_FILENO);
	close(STDIN_FILENO);
	dup2(fd, STDIN_FILENO);

	/*close(fd);
	dup2(infd,STDIN_FILENO);
	close(infd);*/
}

void outputRedirection(tokenlist* tokens, int fileIndex)
{
	int outfd = dup(STDOUT_FILENO); //Duplication STDOUT
	printf("fileindex var: %d\n", fileIndex);
	close(STDOUT_FILENO);
	int fd = open(tokens->items[fileIndex], O_RDWR | O_CREAT, 0600);
	fileIndex = 0;
	dup(outfd);
				// we can close extra reference to stdout
	close(outfd);
}

/*int multipleProcesses(char* token1, char* token2, char* token3)
{
	__pid_t pid = fork();
		int status; 
		if (pid == 0) 
		{
			return execv(token1, [token1, token2, token3]);  //argList OR tokens->items
		}
		else if(pid < 0)
		{
			fprintf(stderr,"Fork failed");
			return 1; 
		}
		else
		{
			waitpid(pid, &status, 0);
			
			printf("INSIDE FUNCTION Child Complete\n");
				//exit(0); 
		}
}*/
