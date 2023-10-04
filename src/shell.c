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

int main()
{
	int commandIndex = 0;	// Keeps track of which token indexes hold commands like "ls" "echo" "cd" etc.
	int fileIndex = 0; 	//Keeps track of index of file
	char *path = NULL;	// When a command token is located like "ls" this will hold its path like "/bin/ls"
	char *argList[10];	// Holds array of c-string arguments that get passed into execv() function. Will look something like {"/bin/ls","-ls",NULL}
	int argCount = 0;	// Increments whenever an argument is found. Resets to 0 when a pipe is encountered.
	bool addArg = false;	// Turns true when an item needs to be added to the argument list.
	char *input = "start";	// Holds user input typed into shell.
	tokenlist *tokens;	// User input is broken into tokens.
	int outfd;
	int fd;
	//tokenlist *argListofTokens;
while (strcmp(input, "exit")!= 0)
{
	// START
	showPrompt();
	input = get_input(); 
	tokens = get_tokens(input);
	//argListofTokens = get_tokens(input); 
//	print(tokens);

	// Loop thru tokens...
	for (int i=0; i<tokens->size; i++)
	{
		// If token is a command like "ls" "echo" etc.
		if (commandIndex == i)
		{
			// Do path search to locate it on the OS.
			tokens->items[0] = get_path_to_command(tokens->items[i]);
			// This variable will now hold the path (if it was found)
			// or it will hold NULL if no path was found.

			if(tokens->items[0]==NULL)
			{
				printf("command not found.\n");
				exit(0);
			}

			// The OS path to the command needs to be first argument in argument list.
			// So we add that now.
			//argList[0] = path; argCount++;
		}

		// Get first character of token...
		switch (tokens->items[i][0])
		{
			case '$' :
				// Replace environment variable.
				tokens->items[i] = replace_environment_variable(tokens->items[i]);
				addArg = true;
				break;
			case '~' :
				// Replace tilda with home folder.
				tokens->items[i] = replace_tilda(tokens->items[i]);
				addArg = true;
				break;
			case '-' :
				addArg = true;
				break;
			case '/' :
				addArg = true;
				break;
			case '|' :
				// If we reach a "pipe" then the next token should be a command token.
				// So, we grab that token index here.
				commandIndex = i+1;
				break;
			case '>' :
				//output redirection
				addArg = true;
				fileIndex = i +1;
				
				break;
			case '<' :
				//input redirection
				addArg = true;
				break;
		}
		// Add token to argument list.
		//if (addArg) {argList[argCount] = tokens->items[i]; argCount++;}
		//addArg = false;
	}
//	print(tokens);

	// Print details before executing command.
//	printf("%s","Executing command...");
	//printf("%s\n",path);
//	printf("%s\n","Using arguments... ");
	//for (int i=0; i<argCount; i++) {printf("%s\n",argList[i]);}
//	printf("%s\n","Here is the output from executing...");

	// Execute command.
	__pid_t pid = fork();
	int status; 
	if (pid == 0) 
	{
		//outfd = dup(STDOUT_FILENO); //Duplication STDOUT
		for(int i = 0; i < tokens->size; i++) printf("%s\n", tokens->items[i]);

		tokens->items[1] = NULL;

		close(STDOUT_FILENO);
        	fd = open("file.txt", O_RDWR | O_CREAT, 0600);
		//fflush(stdout);
		//printf("TESTING CHILD PROCESS");
		execv(tokens->items[0], tokens->items);  //argList OR tokens->items
	}
	else if(pid < 0)
	{
		fprintf(stderr,"Fork failed");
		return 1; 
	}
	else
	{
		waitpid(pid, &status, 0);
		//close(STDOUT_FILENO);
        	//dup2(outfd, STDOUT_FILENO);
        	//close(outfd);
//        	printf("Redirected output.");

		printf("Child Complete\n");
			//exit(0); 
	}

	//execv(path,argList);

	//testing output redirection
	//close(STDOUT_FILENO);	
	//dup2(outfd, STDOUT_FILENO);
	//close(outfd);
	//printf("Redirected output.");

//	print(tokens);
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
