#include <stdio.h>
#include <unistd.h>

#define READ_BUFFER_SIZE 1024


int main() {
	char read_buffer[READ_BUFFER_SIZE];
	char *dir_name = getcwd(NULL, 0);

	do {
		printf("%s$ ", dir_name);
	} while(fgets(read_buffer, sizeof(read_buffer), stdin) != NULL);

	printf("\n");

	return 0;
}
