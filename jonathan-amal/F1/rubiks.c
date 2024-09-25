#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define FACES 6
#define FACE_PIECES 9

#define UP 0 
#define LEFT 1
#define FRONT 2
#define RIGHT 3
#define BACK 4
#define DOWN 5

int cube[FACES][FACE_PIECES] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1},
    {2, 2, 2, 2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3, 3, 3, 3},
    {4, 4, 4, 4, 4, 4, 4, 4, 4},
    {5, 5, 5, 5, 5, 5, 5, 5, 5},
};

void free_tokens(char **tokens, int tokens_count) {
    for (int i = 0; i < tokens_count; i++) {
        free(tokens[i]);
    }

    free(tokens);
}

// TODO: Adapt to kernel
char** split_string(char *str, int *count) {
    int tokens_count = 0;
    bool in_token = false;
    
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == ' ') {
            if (in_token) {
                in_token = false;
            }
        } else {
            if (!in_token) {
                tokens_count++;
                in_token = true;
            }
        }
    }

    char** tokens = malloc(tokens_count * sizeof(char*));
    int token_index = 0;
    int start = -1;
    int length = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == ' ') {
            if (start != -1) {
                tokens[token_index] = malloc((length + 1) * sizeof(char));
                strncpy(tokens[token_index], &str[start], length);
                tokens[token_index][length] = '\0';

                token_index++;
                start = -1;
                length = 0;
            }
        } else {
            if (start == -1) {
                start = i;
            }

            length++;
        }
    }

    if (start != -1) {
        tokens[token_index] = malloc((length + 1) * sizeof(char));
        strncpy(tokens[token_index], &str[start], length);
        tokens[token_index][length] = '\0';
    }

    *count = tokens_count;

    return tokens;
}

bool validate_moves(char *moves) {
    int tokens_count = 0;
    char **tokens = split_string(moves, &tokens_count);

    for (int i = 0; i < tokens_count; i++) {
        // TODO: Check that token is only of size 2, valid character in pos 0, maybe ' in pos 1 
    }

    free_tokens(tokens, tokens_count);
}

void process_moves(char *moves) {
    if (!validate_moves(moves)) {
        return;
    }
}
