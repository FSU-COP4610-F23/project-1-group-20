#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void showPrompt();
void getUserInput();

int main() {

	showPrompt();
	getUserInput();

	//execv("/bin/ls");
	
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

void getUserInput()
{
	char userInput[30];

	fgets(userInput, sizeof(userInput), stdin);

	printf("You typed %s", userInput);
}
