#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <fcntl.h>

const char delimiters[] = " \n\t\v\f\r";


size_t count_tokens(char *str, const char* delim) {
	char *str_cp = strdup(str);
	size_t count = 0;
	char *tok = NULL;

	tok = strtok(str_cp, delim);

	while (tok != NULL) {
		count++;

		tok = strtok(NULL, delim);
	}
	free(str_cp);
	return count;
}

void free_tokens_list(char **tokens, size_t tokens_count)
{
	for(size_t i=0; i<tokens_count;i++)
	{
		free(tokens[i]);
	}
}

size_t create_token_list(char *str, char ***tokens, const char* delim) {
	
	size_t tokens_count = count_tokens(str, delim);

	*tokens = malloc(sizeof(char *) * tokens_count);

	char *tok = NULL;
	size_t index = 0;

	tok = strtok(str, delim);

	while (tok != NULL) {
		(*tokens)[index] = strdup(tok);

		index++;
		tok = strtok(NULL, delim);
	}


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

bool get_input_output_files(char **tokens, size_t tokens_count, char **in_file, char **out_file) {
	// TODO: What happens in case of < < ?
	if (strcmp(tokens[tokens_count - 1], "<") == 0 || strcmp(tokens[tokens_count - 1], ">") == 0) {
		fprintf(stderr, "No file provided for redirection.");

		return false;
	}

	for(size_t i = 0; i < tokens_count; i++) {
		if (strcmp(tokens[i], "<") == 0) {
			*in_file = tokens[i + 1];
		}

		if (strcmp(tokens[i], ">") == 0) {
			*out_file = tokens[i + 1];
		}
	}

	return true;
}

pid_t fork_and_exec(char **tokens, size_t tokens_count)
{

	pid_t pid = fork();

	if (pid == 0) {
		run_execv(tokens, tokens_count);

		exit(0);

	} else {
		waitpid(pid, NULL, 0);
	}

	return pid;
}

char* find_file_path_env(char* file)
{
	char* delim = ":";
	char** dirs;
	char* env = getenv("PATH");
	char * env_cp = strdup(env);
	size_t dirs_count = create_token_list(env_cp,&dirs, delim);
	
	if(!dirs || !file)
		return NULL;
	
	char* new_path = NULL;
	size_t dirs_len;
	size_t file_len = strlen(file);
	
	for(size_t i=0; i<dirs_count;i++)
	{
		dirs_len = strlen(dirs[i]);
		new_path = malloc(dirs_len + file_len + 2);

		strcpy(new_path, dirs[i]);

		new_path[dirs_len] = '/';
		strcpy(new_path+dirs_len+1, file);
		new_path[dirs_len+1+file_len] = '\0';
	 
		struct stat sb;
		if(stat(new_path,&sb) == 0 && S_ISREG(sb.st_mode) && (sb.st_mode & S_IXUSR))
		{
			break;
		}
		else
		{
			free(new_path);
			new_path = NULL;
		}
	}
	
	free(env_cp);
	free_tokens_list(dirs,dirs_count);
	free(dirs);
	return new_path; 
}

void sub_home_dir(char **tokens, size_t tokens_count)
{
	char* env = getenv("HOME");
	char * env_cp = strdup(env);
	size_t len = 0;
	size_t home_len = strlen(env_cp);
	for(size_t i=0; i<tokens_count; i++)
	{
		if(tokens[i][0]=='~')
		{
			//sub
			len = strlen(tokens[i]);
			if(len == 1){
				//~
				free(tokens[i]);
				tokens[i] = strdup(env_cp);
			}
			else if(len >1 && tokens[i][1]=='/'){
				//~/whatever
				char* tmp = malloc(sizeof(char)*(len+home_len+1));
				strcpy(tmp, env_cp);
				strcpy(tmp+home_len,tokens[i]+1);
				free(tokens[i]);
				tokens[i]= tmp;
			}
			else{
				//locate user
				char* usr_name_end = strchrnul(tokens[i],'/');
				size_t usr_name_len = usr_name_end - tokens[i] -1 ;
				if(usr_name_len == 0) {
					continue;
				}
				char* usr_name = strndup(tokens[i]+1,usr_name_len);
				if(!usr_name){
					free(usr_name);
					continue;
				}
				struct passwd *pw = getpwnam(usr_name);
				if(!pw)	{
					free(usr_name);
					continue;
				}
				
				size_t pw_dir_len = strlen(pw->pw_dir);
				size_t tmp_len = len - usr_name_len + pw_dir_len -1 ;
				char* tmp = malloc(tmp_len+1);
				strcpy(tmp, pw->pw_dir);
				strcpy(tmp+pw_dir_len, tokens[i]+usr_name_len+1);
				tmp[tmp_len] = '\0';
				free(usr_name);
				free(tokens[i]);
				tokens[i]= tmp;

			}
		}
	}
	free(env_cp);
}

char* add_redirection_padding(const char* input) {
    size_t length = strlen(input);
    size_t new_length = length * 3; 

    char* result = malloc(new_length + 1);

    size_t j = 0;
    for (size_t i = 0; i < length; ++i) {
        if (input[i] == '<' || input[i] == '>') {
            if (i > 0 && input[i - 1] != ' ') {
                result[j++] = ' '; 
            }

            result[j++] = input[i];

            if (i < length - 1 && input[i + 1] != ' ') {
                result[j++] = ' '; 
            }

        } else {
            result[j++] = input[i];
        }
    }

    result[j] = '\0';

    return result;
}

size_t find_first_redirection(char **tokens, size_t tokens_count) {
	for (size_t i = 0; i < tokens_count; i++) {
		if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0) {
			return i;
		}
	}

	return tokens_count;
}

void parse_input(char *str, char **dir_name) {
	if (strcmp(str, "\n") == 0) {
		return;
	}

	char *new_str = add_redirection_padding(str);

	char **tokens;
	size_t tokens_count = create_token_list(new_str, &tokens, delimiters);

	if (tokens_count == 0) {
		return;
	}

	sub_home_dir(tokens, tokens_count);

	char *in_file = NULL;
	char *out_file = NULL;

	if (!get_input_output_files(tokens, tokens_count, &in_file, &out_file)) {
		return;
	}

	int file_permissions = 0666;
	int file_mode = O_CREAT | O_RDWR | O_TRUNC;

	int stdout_copy = dup(STDOUT_FILENO);
	int stdin_copy = dup(STDIN_FILENO);

	// TODO: handle open errors
	if (in_file != NULL) {
		close(STDIN_FILENO);
		open(in_file, O_RDWR, file_permissions);
	}

	if (out_file != NULL) {
		close(STDOUT_FILENO);
		open(out_file, file_mode, file_permissions);
	}

	size_t args_count = find_first_redirection(tokens, tokens_count);

	if (strcmp(tokens[0], "exit") == 0) {
		if (args_count > 1) {
			fprintf(stderr, "Exit doesn't take arguments\n");

		} else {
			free_tokens_list(tokens, tokens_count);
			free(tokens);
			free(str);
			free(new_str);
			free(*dir_name);

			exit(0);
		}	

	} else if (strcmp(tokens[0], "cd") == 0) {
		if (args_count != 2) {
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
		if (args_count < 2) {
			fprintf(stderr, "exec should take atleast one argument of the program to run.\n");

		} else {
			run_execv(tokens + 1, args_count - 1);
		}

    } else if (tokens[0][0] == '.' || tokens[0][0] == '/') {
		fork_and_exec(tokens, args_count);

	} else {
		char* path = find_file_path_env(tokens[0]);
		if(!path){
			fprintf(stderr, "Unrecognized command: %s\n", tokens[0]);
		}else{
			free(tokens[0]);
			tokens[0] = path;
			fork_and_exec(tokens, args_count);
			//free(path);
		}
	}

	dup2(stdin_copy, STDIN_FILENO);
	dup2(stdout_copy, STDOUT_FILENO);

	close(stdin_copy);
	close(stdout_copy);

	free(new_str);
	free_tokens_list(tokens,tokens_count);
	free(tokens);
}

int main() {
	char *string = NULL;
	size_t size = 0;
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
