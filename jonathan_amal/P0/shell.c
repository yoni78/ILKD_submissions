#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

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

void run_execv(char **tokens, size_t tokens_count) {
	char *args[tokens_count + 1];

	for(size_t i = 0; i < tokens_count; i++) {
		args[i] = tokens[i];
	}

	args[tokens_count] = NULL;

	int res = execv(tokens[0], args);

	if (res == -1) {
		fprintf(stderr, "execv error: %s\n", strerror(errno)); 
	}
}

void parse_input(char *str, char **dir_name) {
	if (strcmp(str, "\n") == 0) {
		return;
	}

	char **tokens;
	size_t tokens_count = create_token_list(str, &tokens);

	if (tokens_count == 0) {
		return;
	}

	if (strcmp(tokens[0], "exit") == 0) {
		if (tokens_count > 1) {
			fprintf(stderr, "Exit doesn't take arguments\n");

		} else {
			free(tokens);
			free(str);
			free(*dir_name);

			exit(0);
		}	

	} else if (strcmp(tokens[0], "cd") == 0) {
		if (tokens_count != 2) {
			fprintf(stderr, "cd should take one argument exactly of the directory to change to.\n");
			
		} else {
			int res = chdir(tokens[1]);

			if(res != 0) {
				fprintf(stderr, "cd error: %s\n", strerror(errno));

			} else {
				free(*dir_name);

				*dir_name = getcwd(NULL, 0);
			}
		}

	} else if (strcmp(tokens[0], "exec") == 0) {
		if (tokens_count < 2) {
			fprintf(stderr, "exec should take atleast one argument of the program to run.\n");

		} else {
			run_execv(tokens + 1, tokens_count - 1);
		}

    } else if (tokens[0][0] == '.' || tokens[0][0] == '/') {
		pid_t pid = fork();

		if (pid == 0) {
			run_execv(tokens, tokens_count);

			exit(0);

		} else {
			waitpid(pid, NULL, 0);
		}

	} else {
		fprintf(stderr, "Unrecognized command: %s\n", tokens[0]);
	}

	free(tokens);
}

int main() {
	char *string = NULL;
	size_t size = 0;
	// ssize_t chars_read = 0;

	char *dir_name = getcwd(NULL, 0);

	printf("%s$ ", dir_name);

	while(getline(&string, &size, stdin) != -1) {
		parse_input(string, &dir_name);

		printf("%s$ ", dir_name);
	}

	printf("\n");

	if (string != NULL) {
		free(string);
	}

	free(dir_name);

	return 0;
}
