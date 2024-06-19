#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define READ_BUFFER_SIZE 1024
#define BUILT_IN_COMMANDS_NUM 3

const char delimiters[] = " \n\t\v\f\r";
const char* builtin_commands[BUILT_IN_COMMANDS_NUM] = {"exit", "cd", "exec"};

void parse_input(char buf[READ_BUFFER_SIZE]) {
	if (strcmp(buf, "\n") == 0) {
		return;
	}

	char *tok = strtok(buf, delimiters);

	fprintf(stderr, "Unrecognized command: %s\n", tok);
}

int main() {
	char read_buffer[READ_BUFFER_SIZE];
	char *dir_name = getcwd(NULL, 0);

	printf("%s$ ", dir_name);

	while(fgets(read_buffer, sizeof(read_buffer), stdin) != NULL) {
		parse_input(read_buffer);

		printf("%s$ ", dir_name);
	}

	printf("\n");

	return 0;
}
