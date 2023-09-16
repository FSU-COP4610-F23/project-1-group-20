#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	char *str1 = getenv("USER");
	char *str2 = getenv("MACHINE");
	char *str3 = getenv("PWD");

	printf("%s@", str1);
	printf("%s:", str2);
	printf("%s>", str3);
	return 0;
}
