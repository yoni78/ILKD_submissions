#include <stdio.h>
#include <unistd.h>

int main() {
	char *dir_name = getcwd(NULL, 0);

	printf("%s$ \n", dir_name);
}
