#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

// Each row has 32 cells + 2 walls + new line
// Each col has 16 cells + 2 walls
#define GRID_STRING_SIZE 35 * 18 

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
                    "----------------------------------\n" \

#define GRID1        "----------------------------------\n" \
                    "|*                               |\n" \
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

#define GRID2        "----------------------------------\n" \
                    "|**                              |\n" \
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

#define GRID3        "----------------------------------\n" \
                    "| *                              |\n" \
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

#define GRID1_LIVE_CELL_OTHER_CHAR \
                    "----------------------------------\n" \
                    "|$                               |\n" \
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

#define GRID_LIVE_CORNER_SQUARE \
                    "----------------------------------\n" \
                    "|**                              |\n" \
                    "|**                              |\n" \
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

#define GRID_DEAD_STAYS_DEAD \
                    "----------------------------------\n" \
                    "|*                               |\n" \
                    "|**                              |\n" \
                    "| *                              |\n" \
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

#define GRID_OVERPOPULATION_DIES \
                    "----------------------------------\n" \
                    "|* *                             |\n" \
                    "|* *                             |\n" \
                    "| *                              |\n" \
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

#define GRID_STABLE_SMALL \
                    "----------------------------------\n" \
                    "| *                              |\n" \
                    "|* *                             |\n" \
                    "| *                              |\n" \
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

#define GRID_STABLE_LARGE \
                    "----------------------------------\n" \
                    "| **                             |\n" \
                    "|*  *                            |\n" \
                    "| ** *                           |\n" \
                    "|    *                           |\n" \
                    "|    **                          |\n" \
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

#define GRID_BLINKER_1 \
                    "----------------------------------\n" \
                    "|                                |\n" \
                    "|***                             |\n" \
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

#define GRID_BLINKER_2 \
                    "----------------------------------\n" \
                    "| *                              |\n" \
                    "| *                              |\n" \
                    "| *                              |\n" \
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
#define TESTS_NUM 20 

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

bool test_one_write() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    write(fd, NULL, 0);
    read(fd, buf, 0);
    bool result = strcmp(buf, GRID1) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_write_with_count() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    write(fd, NULL, 10);
    read(fd, buf, 0);
    bool result = strcmp(buf, GRID1) == 0;

    if (result) {
        close(fd);
    }

    return result;
}


bool test_two_writes() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    write(fd, NULL, 0);
    write(fd, NULL, 0);
    read(fd, buf, 0);
    bool result = strcmp(buf, GRID2) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_lseek_without_write() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 0, SEEK_SET);
    read(fd, buf, 0);
    bool result = strcmp(buf, EMPTY_GRID) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_lseek_bounds() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 4096, SEEK_SET);
    read(fd, buf, 0);
    bool result = strcmp(buf, EMPTY_GRID) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_lseek_and_one_write_without_offset() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 0, SEEK_SET);
    write(fd, NULL, 0);
    read(fd, buf, 0);
    bool result = strcmp(buf, GRID1) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_lseek_and_two_writes_without_offset() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 0, SEEK_SET);
    write(fd, NULL, 0);
    write(fd, NULL, 0);
    read(fd, buf, 0);
    bool result = strcmp(buf, GRID2) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_lseek_and_toggle_undo() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    write(fd, NULL, 0);
    lseek(fd, -1, SEEK_CUR);
    write(fd, NULL, 0);
    read(fd, buf, 0);
    bool result = strcmp(buf, EMPTY_GRID) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_lseek_and_one_write_with_offset() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 1, SEEK_SET);
    write(fd, NULL, 0);
    read(fd, buf, 0);
    bool result = strcmp(buf, GRID3) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_ioctl_with_invalid_op() {
    int fd = open(FILE_PATH, O_RDWR);

    ioctl(fd, 2);

    bool result = errno == EPERM;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_changing_live_cell_character() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 0, SEEK_SET);
    write(fd, NULL, 0);

    ioctl(fd, 0, '$');

    read(fd, buf, 0);

    bool result = strcmp(buf, GRID1_LIVE_CELL_OTHER_CHAR) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_changing_live_cell_character_to_bad_value() {
    int fd = open(FILE_PATH, O_RDWR);

    ioctl(fd, 0, 0x20);

    bool result = errno == EINVAL;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_cell_without_neighbors_dies() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 0, SEEK_SET);
    write(fd, NULL, 0);

    ioctl(fd, 1);

    read(fd, buf, 0);
    bool result = strcmp(buf, EMPTY_GRID) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_cell_with_3_neighbors_becomes_live() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 1, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 32, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 33, SEEK_SET);
    write(fd, NULL, 0);

    ioctl(fd, 1);

    read(fd, buf, 0);
    bool result = strcmp(buf, GRID_LIVE_CORNER_SQUARE) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_cell_with_4_neighbors_stays_dead() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 0, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 32, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 33, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 34, SEEK_SET);
    write(fd, NULL, 0);

    ioctl(fd, 1);

    read(fd, buf, 0);
    bool result = strcmp(buf, GRID_DEAD_STAYS_DEAD) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_cell_with_4_neighbors_dies() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 0, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 1, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 32, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 33, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 34, SEEK_SET);
    write(fd, NULL, 0);

    ioctl(fd, 1);

    read(fd, buf, 0);
    bool result = strcmp(buf, GRID_OVERPOPULATION_DIES) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_cell_with_2_neighbors_remains_alive() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 1, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 32, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 34, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 65, SEEK_SET);
    write(fd, NULL, 0);

    ioctl(fd, 1);

    read(fd, buf, 0);
    bool result = strcmp(buf, GRID_STABLE_SMALL) == 0;

    if (result) {
        close(fd);
    }

    return result;
}

bool test_large_stable_structure() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 1, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 2, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 32, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 35, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 65, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 66, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 68, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 100, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 132, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 133, SEEK_SET);
    write(fd, NULL, 0);

    ioctl(fd, 1);

    read(fd, buf, 0);
    bool result = strcmp(buf, GRID_STABLE_LARGE) == 0;

    ioctl(fd, 1);

    read(fd, buf, 0);
    result = result && (strcmp(buf, GRID_STABLE_LARGE) == 0);

    ioctl(fd, 1);

    read(fd, buf, 0);
    result = result && (strcmp(buf, GRID_STABLE_LARGE) == 0);

    if (result) {
        close(fd);
    }

    return result;
}

bool test_oscillating_structure() {
    int fd = open(FILE_PATH, O_RDWR);

    char buf[GRID_STRING_SIZE];

    lseek(fd, 32, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 33, SEEK_SET);
    write(fd, NULL, 0);

    lseek(fd, 34, SEEK_SET);
    write(fd, NULL, 0);

    read(fd, buf, 0);
    bool result = strcmp(buf, GRID_BLINKER_1) == 0;

    ioctl(fd, 1);

    read(fd, buf, 0);
    result = result && (strcmp(buf, GRID_BLINKER_2) == 0);

    ioctl(fd, 1);

    read(fd, buf, 0);
    result = result && (strcmp(buf, GRID_BLINKER_1) == 0);

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
    print_result(test_one_write(), test_num++, "Toggle one cell");
    print_result(test_write_with_count(), test_num++, "Toggle one cell- write is irrelevant");
    print_result(test_two_writes(), test_num++, "Toggle Two cells");
    print_result(test_lseek_and_one_write_without_offset(), test_num++, "Toggle first cell after lseek");
    print_result(test_lseek_and_two_writes_without_offset(), test_num++, "Toggle cells (0,0) and (0,1) after lseek");
    print_result(test_lseek_and_toggle_undo(), test_num++, "Toggle same cell twice using lseek");
    print_result(test_lseek_and_one_write_with_offset(), test_num++, "Toggle (0,1) after lseek");
    print_result(test_ioctl_with_invalid_op(), test_num++, "Ioctl with invalid op");
    print_result(test_changing_live_cell_character(), test_num++, "Change live cells character");
    print_result(test_changing_live_cell_character_to_bad_value(), test_num++, "Change live cells character to a bad value");
    print_result(test_cell_without_neighbors_dies(), test_num++, "Cell without neighbor dies");
    print_result(test_cell_with_3_neighbors_becomes_live(), test_num++, "Dead cell with 3 neighbors becomes alive");
    print_result(test_cell_with_4_neighbors_stays_dead(), test_num++, "Dead cell with 4 neighbors stays dead");
    print_result(test_cell_with_4_neighbors_dies(), test_num++, "Live cell with 4 neighbors dies");
    print_result(test_cell_with_2_neighbors_remains_alive(), test_num++, "Live cell with 2 neighbors remains alive");
    print_result(test_large_stable_structure(), test_num++, "Large stable structure");
    print_result(test_oscillating_structure(), test_num++, "Oscillating structure");
    // TODO: test lseek out of bounds 
    // TODO: test weird values like in instructions
    return 0;
}