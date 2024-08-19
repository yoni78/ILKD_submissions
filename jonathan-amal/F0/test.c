#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// Each row has 32 cells + 2 walls + new line
// Each col has 32 cells + 2 walls
#define GRID_STRING_SIZE 35 * 34

#define EMPTY_GRID  "----------------------------------\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "|                                |\n" \
                    "----------------------------------\n" \
// TODO: How to get input file?
// #define FILE_PATH "/dev/game_of_life"
#define FILE_PATH "/dev/null"
#define TESTS_NUM 3

void print_tests_num(int num) {
    printf("1..%d\n", num);
}

void print_result(bool success, int test_num, char *message) {
    if (!success) {
        printf("not ");
    }

    printf("ok %d - %s\n", test_num, message);
}

bool test_file_exists() {
    int fd = open(FILE_PATH, O_RDWR);

    bool result = fd >= 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_open_twice_fails() {
    int fd1 = open(FILE_PATH, O_RDWR);
    open(FILE_PATH, O_RDWR);

    bool result = errno == EBUSY;

    if (result) {
        close(fd1);
    }

    return result;
}

bool test_open_returns_empty_grid() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    read(fd, buf, 0);

    bool result = strcmp(buf, EMPTY_GRID) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

int main() {
    print_tests_num(TESTS_NUM);

    int test_num = 1;

    print_result(test_file_exists(), test_num++, "File exists");
    print_result(test_open_twice_fails(), test_num++, "Can't open file twice");
    print_result(test_open_returns_empty_grid(), test_num++, "Open returns empty grid");

    return 0;
}