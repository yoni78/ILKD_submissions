#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "rubiks.h"

static int test_counter = 0;

#define RUN_TEST(test_func) do {                 \
    test_counter++;                              \
    bool result = (test_func)();                 \
    printf("Test %d: %s\n", test_counter,        \
           result ? "PASSED" : "FAILED");        \
} while (0)

bool split_string_test() {
    char *s = " abc  def 123  ";

    int tokens_count = 0;
    char **res = split_string(s, &tokens_count);

    if (tokens_count != 3) {
        return false;
    }

    if (strcmp(res[0], "abc") == 1) {
        return false;
    }

    if (strcmp(res[1], "def") == 1) {
        return false;
    }

    if (strcmp(res[2], "123") == 1) {
        return false;
    }

    return true;
}

int main() {
    RUN_TEST(split_string_test);
}