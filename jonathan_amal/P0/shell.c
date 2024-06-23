#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
	char *dir_name = getcwd(NULL, 0);

	printf("%s$ ", dir_name);
	printf("\n");

	return 0;
}
