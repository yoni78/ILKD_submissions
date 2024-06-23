#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


const char delimiters[] = " \n\t\v\f\r";

size_t count_tokens(char *str) {
	size_t count = 0;
	char *tok = NULL;

	tok = strtok(str, delimiters);

	while (tok != NULL) {
		count++;

		tok = strtok(NULL, delimiters);
	}

	return count;
}

size_t create_token_list(char *str, char ***tokens) {
	size_t str_len = strlen(str);
	char *str_cp = malloc(sizeof(char) * str_len + 1);
	strcpy(str_cp, str);

	size_t tokens_count = count_tokens(str_cp);

	*tokens = malloc(sizeof(char *) * tokens_count);

	char *tok = NULL;
	size_t index = 0;

	tok = strtok(str, delimiters);

	while (tok != NULL) {
		(*tokens)[index] = tok;

		index++;
		tok = strtok(NULL, delimiters);
	}

	free(str_cp);

	return tokens_count;
}

void parse_input(char *str) {
	if (strcmp(str, "\n") == 0) {
		return;
	}

	char **tokens;
	size_t tokens_count = create_token_list(str, &tokens);
	//create_token_list(str, &tokens);
	if (tokens_count == 0) {
		return;
	}
	fprintf(stderr, "Unrecognized command: %s\n", tokens[0]);

	free(tokens);
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

	free(dir_name);

	return 0;
}
