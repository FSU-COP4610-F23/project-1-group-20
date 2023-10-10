#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "lexer.h"

void showPrompt(); //show user prompt and directory
void print(tokenlist *tokens); //printing token list
char* replace_environment_variable(char *token); //replacing env with neccessary environment
//printing background processes and states
void printJobs(tokenlist *tokens, int num, __pid_t BackgroundProcesses[10], int runningOrDone[10]); 
char* replace_tilda(char *token); //Replaces tilde with the path to the path to the  user's home directory. 
char* get_path_to_command(char* command); //Returns the path to where the specificed command is located.
tokenlist* convert_tokens(tokenlist* tokens); //Performs initial token conversions.
int setenv(const char *name, const char *value, int overwrite); //Explicitly set environment. 

struct commandTable
{
	tokenlist* items[10];
	int size;
};

//////////////////////////////// MAIN //////////////////////////////
int main()
{
	int fileIndex = 0; 			// Keeps track of index of file
	char *input = "start";			// Holds user input typed into shell.
	tokenlist *tokens;			// User input is broken into tokens.
	int inputRedirect = 0; 			// Check if we use input redirection < 
	int outputRedirect = 0; 		// Check if we use output redirection >
	struct commandTable ct; ct.size=0;	// Command table holds tokens for each process.
	__pid_t pid;				// Process id.
	int status;				// Status of process.
	int fd[2];				// Holds 2 file descriptors for pipe in & pipe out.
	pipe(fd);				// Make pipe.
	char cwd[200];				// Current working directory of shell. Updated afer a "cd"
	bool internalCommand = false;		// To identify internal shell commands vs external commands.
	__pid_t BackgroundProcesses[10];
	int numBackgroundProcesses = 0;
	int ampersandIndex = 0; 
	struct commandTable jobs; jobs.size=0; 
	int checkBackgroundProcess = 0; 
	int counter = 1; 
	int runningOrDone[10] = {0,0,0,0,0,0,0,0,0,0}; //checks if background process still running or done

	// START Loop till user types "exit"
	while (strcmp(input, "exit") != 0)
	{
		showPrompt();
		input = get_input(); 

		// User typed exit?
		if (strcmp(input, "exit") == 0)
		{
			// Print valid commands.
			exit(0);
		}

		// Tokenize user input.
		tokens = get_tokens(input);

		//THIS IS NEW
		for (int i=0; i<tokens->size; i++)
		{
			switch (tokens->items[i][0])
			{
				case '&' :
				jobs.items[jobs.size] = tokens;
				jobs.size++;
				break;
			}
		}

		// Convert/Expand tokens like $ or ~ or command path.
		tokens = convert_tokens(tokens);

		// User typed cd?
		if (strcmp(tokens->items[0], "/bin/cd") == 0)
		{
			if (tokens->size == 1) {chdir(getenv("HOME"));}
			else {chdir(tokens->items[1]);}
			getcwd(cwd,200);
			setenv("PWD",cwd,1);
			internalCommand = true;
		}
		// User typed jobs?
		else if (strcmp(tokens->items[0], "/bin/jobs") == 0)
		{
			for (int i=0; i<jobs.size; i++) {printJobs(jobs.items[i], i+1,BackgroundProcesses, runningOrDone);}
			internalCommand = true;
		}

		// Build command table.
		int i = 0;
		while (i<tokens->size)
		{
			bool done=false;
			tokenlist *processTokens = new_tokenlist();
			while (!done && i<tokens->size)
			{
				switch (tokens->items[i][0])
				{
					case '|' :
						done=true; tokens->items[i] = NULL; break;
					default :
						add_token(processTokens, tokens->items[i]);
				}
				i++;
			}
			// Add process tokens to next item in the command table.
			ct.items[ct.size] = processTokens;
			ct.size++;
		}

	// EXTERNAL command execution.
	if (!internalCommand)
	{	
		if (ct.size == 1)
		{
			tokens = ct.items[0]; //setting tokens to contents of the command table
			inputRedirect = outputRedirect = 0;
			// Find out if input or output redirection is needed.
			for (int i=0; i<tokens->size; i++)
			{
				// Get first character of token...
				switch (tokens->items[i][0])
				{
					case '>' :
						//output redirection
						fileIndex = i + 1;
						outputRedirect = 1 ;
						break;
					case '<' :
						//input redirection
						fileIndex = i + 1; 
						inputRedirect = 1; 
						break;

					case '&' :
						//identified for background processes
						ampersandIndex = i; 
						numBackgroundProcesses++;
						tokens->items[ampersandIndex] = NULL;
						checkBackgroundProcess = 1; 
						break; 
				}
			}
			// REDIRECTION.
			pid = fork();
 
			if (pid == 0) 
			{
				if (inputRedirect == 1)
				{ 
					//if input token is identified
					tokens->items[fileIndex-1] = NULL;
					dup(STDIN_FILENO);
					close(STDIN_FILENO);
					fd[1] = open(tokens->items[fileIndex], O_RDONLY);
				}
				if (outputRedirect == 1)
				{
					//if output token is identified
					tokens->items[fileIndex-1] = NULL;
					dup(STDOUT_FILENO);
					close(STDOUT_FILENO);
					fd[0] = open(tokens->items[fileIndex], O_RDWR | O_CREAT, 0600);
				}
				// Execute command.
				execv(tokens->items[0], tokens->items);
			}
			else
			{
				//if a background process exisits
				if (checkBackgroundProcess > 0)
				{
					BackgroundProcesses[numBackgroundProcesses-1] = pid;

					for (int i = 0; i < numBackgroundProcesses; i++)
					{
						pid_t result = waitpid(BackgroundProcesses[i], &status, WNOHANG);
						if (result == 0) {
							// checks if background process still running
							//printf("%d Background process is still running.\n", result);
							
						} else if (result > 0) {
						
							if (WIFEXITED(status)) {
							
								printf("[%d]  + %d done \n",counter++, result);
								//loop thru pids to check which is result, if 
								for (int j = 0; j<numBackgroundProcesses;j++)
								{
									if(BackgroundProcesses[j] == result)
									{
										checkBackgroundProcess--;
										runningOrDone[j] = 1;
									}
								}
								//printf("[%d] %d \n",numBackgroundProcesses, BackgroundProcesses[numBackgroundProcesses-1]);
							} 
						} 
					}

					printf("[%d] %d \n",numBackgroundProcesses, BackgroundProcesses[numBackgroundProcesses-1]);


					//checkBackgroundProcess = 0; 


				}
				else{waitpid(pid, &status, 0);} 

			}	
		}
		else
		{
			// PIPE.
			// Loop thru command table.
			for (int i = 0; i < ct.size; i++)
			{
				pid = fork();
				
				if (pid == 0)
				{
					//child process
					if (i != 0)
					{
						dup2(fd[0], STDIN_FILENO);
						close(fd[0]);
						close(fd[1]);
					}

					if (i != ct.size - 1)
					{
						dup2(fd[1], STDOUT_FILENO);
						close(fd[0]);
						close(fd[1]);
					}

				execv(ct.items[i]->items[0], ct.items[i]->items);
				//child process exits
				}
				else
				{
					//parent process
					if (i != 0)
					{
						//close read
						close(fd[0]);
					}

					if (i != ct.size - 1)
					{
						//close write
						close(fd[1]);
					}

					//waiting for child process
					waitpid(pid, &status, 0);
				}
			}
		}
}
	// Reset and get next user input.
	free(input);
	free_tokens(tokens);
	ct.items[0] = NULL;
	ct.size = 0;
	internalCommand = false;
	}
// exit.
return 0;
}

//////////////////////////////////////// FUNCTIONS //////////////////////////////////////
// Function prints a command-line prompt similar to that of a Unix shell.
void showPrompt()
{
	//variables for obtaining user, machine, or pwd
	char *str1 = getenv("USER");
	char *str2 = getenv("MACHINE");
	char *str3 = getenv("PWD");
	printf("%s@", str1);
	printf("%s:", str2);
	printf("%s>", str3);
}

// Prints tokens in token list, allowing for debugging.
void print(tokenlist *tokens)
{
	printf("Tokenlist...\n");
	for (int i=0; i<tokens->size; i++) {printf("%s\n",tokens->items[i]);}
}

void printJobs(tokenlist *tokens, int num, __pid_t BackgroundProcesses[10], int runningOrDone[10])
{	
	//make sure to delete
	if (runningOrDone[num-1] == 1)
	{
		return; 
		printf("[%d]   + %d done ",num,BackgroundProcesses[num-1]);
	} 
	printf("[%d]   + %d running ",num,BackgroundProcesses[num-1]);
	for (int i=0; i<tokens->size; i++)
	{
		printf("%s ",tokens->items[i]);
	}
	printf("\n");
}

// Creating a new char array to remove the leading character, such as the '$'.
// Dynamically allocated new memory for the new array with a size that can hold the
// environment variable. Uses 'getenv' to obtain the value of the environment variable.
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

// This function takes a token as input and replaces the tilde with the path to the
// users home directory.
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
	char *p;
	char *paths;
	paths = (char*)malloc(strlen(getenv("PATH")+1));
	strcpy(paths,getenv("PATH"));
	char *delimitedPATH = strtok(paths,":");
	while (delimitedPATH != NULL)
	{	
		p = (char*)malloc(100);

		// Append the command we are searching for to end of path.
		strcpy(p,delimitedPATH);
		strcat(p,"/");
		strcat(p,command);

		// If path is accessible, return it.
		if (access(p, F_OK) == 0) {return p;}

		// Otherwise, move to next path.
		delimitedPATH = strtok(NULL,":");
		p = NULL;
	}
	// If we've gotten here, command was not found at any path. Return NULL.
	return p;
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
