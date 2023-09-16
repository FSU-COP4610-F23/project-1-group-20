#include <stdio.h>

int main()
{
	char userInput[30];

	printf("Type something...\n");
	fgets(userInput, sizeof(userInput), stdin);
	printf("You typed: %s", userInput);

	return 0;
}
