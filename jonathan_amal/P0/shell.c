#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


void parse_input(char *str) {
	if (strcmp(str, "\n") == 0) {
		return;
	}

	fprintf(stderr, "Unrecognized command\n");
}

int main() {
	char *string = NULL;
	size_t size = 0;
	// ssize_t chars_read = 0;

	char *dir_name = getcwd(NULL, 0);

	printf("%s$ ", dir_name);

	while(getline(&string, &size, stdin) != -1) {
		parse_input(string);

		printf("%s$ ", dir_name);
	}

	printf("\n");

	if (string != NULL) {
		free(string);
	}

	return 0;
}
