#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "rubiks.h"

void print_faces();

#define UP 0 
#define LEFT 1
#define FRONT 2
#define RIGHT 3
#define BACK 4
#define DOWN 5

static int cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}},
    {{2, 2, 2}, {2, 2, 2}, {2, 2, 2}},
    {{3, 3, 3}, {3, 3, 3}, {3, 3, 3}},
    {{4, 4, 4}, {4, 4, 4}, {4, 4, 4}},
    {{5, 5, 5}, {5, 5, 5}, {5, 5, 5}},
};

static int solved_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}},
    {{2, 2, 2}, {2, 2, 2}, {2, 2, 2}},
    {{3, 3, 3}, {3, 3, 3}, {3, 3, 3}},
    {{4, 4, 4}, {4, 4, 4}, {4, 4, 4}},
    {{5, 5, 5}, {5, 5, 5}, {5, 5, 5}},
};

//UP, R, D L of each face (in that order)
int adjacent [FACES][FACES-2] = {
    {4,3,2,1},
    {0,2,5,4},
    {0,3,5,1},
    {0,4,5,2},
    {0,1,5,3},
    {2,3,4,1},
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

bool is_valid_move(char move) {
    return move == 'F' || move == 'R' || move == 'U' || move == 'L' || move == 'B' || move == 'D';
}

bool validate_moves(char *moves) {
    int tokens_count = 0;
    char **tokens = split_string(moves, &tokens_count);
    bool is_valid = true;

    for (int i = 0; i < tokens_count; i++) {
        size_t token_size = strlen(tokens[i]);

        bool bad_token_size = !(token_size == 1 || token_size == 2);
        bool bad_regular_move = !is_valid_move(tokens[i][0]);
        bool bad_reverse_move = (token_size == 2) && (tokens[i][1] != '\'');

        if (bad_token_size || bad_regular_move || bad_reverse_move) {
            is_valid = false;
            break;
        }
    }

    free_tokens(tokens, tokens_count);

    return is_valid;
}

int get_face_index(char c)
{
    switch(c) {
        case 'U': return 0; // Up
        case 'L': return 1; // Left
        case 'F': return 2; // Front
        case 'R': return 3; // Right
        case 'B': return 4; // Back
        case 'D': return 5; // Down
        default: return -1; // Invalid input
    }
}

void rotate_clockwise(int face_index);
void rotate_anticlockwise(int face_index);
void rotate_clockwise_adjacent_faces_front();
void rotate_clockwise_adjacent_faces_up();
void rotate_clockwise_adjacent_faces_right();
void rotate_clockwise_adjacent_faces_back();
void rotate_clockwise_adjacent_faces_left();
void rotate_clockwise_adjacent_faces_down();

void rotate_anticlockwise_adjacent_faces_front();
void rotate_anticlockwise_adjacent_faces_up();
void rotate_anticlockwise_adjacent_faces_right();
void rotate_anticlockwise_adjacent_faces_back();
void rotate_anticlockwise_adjacent_faces_left();
void rotate_anticlockwise_adjacent_faces_down();


void exec_move(char* move)
{
    size_t token_size = strlen(move);
    int face_index = get_face_index(move[0]);

    if(token_size == 1)
        rotate_clockwise(face_index);
    else if (token_size == 2)
        rotate_anticlockwise(face_index);
    else
        printf("token: this should never happen");
}

void process_moves(char *moves) {
    if (!validate_moves(moves)) {
        return;
    }
    int tokens_count = 0;
    char **tokens = split_string(moves, &tokens_count);

    for(int i=0; i< tokens_count; i++)
    {

    
        exec_move(tokens[i]);
    }
    free_tokens(tokens, tokens_count);
}



void print_face(int face[CUBE_SIZE][CUBE_SIZE]) {
    for (int i = 0; i < CUBE_SIZE; i++) {
        for (int j = 0; j < CUBE_SIZE; j++) {
            printf("%d ", face[i][j]);
        }
        printf("\n");
    }
}

void rotate_clockwise(int face_index)
{

    int (*face)[CUBE_SIZE] = cube[face_index];
    int temp[CUBE_SIZE][CUBE_SIZE];

    // Copy the face to a temporary array
    for (int i = 0; i < CUBE_SIZE; i++) {

        for (int j = 0; j < CUBE_SIZE; j++) {
            temp[i][j] = face[i][j];
        }
    }
    // Rotate the face 90 degrees clockwise
    for (int i = 0; i < CUBE_SIZE; i++) {
        for (int j = 0; j < CUBE_SIZE; j++) {
            face[j][CUBE_SIZE - 1 - i] = temp[i][j];
        }
    }

    switch(face_index) {
        case 0: // Up
            rotate_clockwise_adjacent_faces_up();
            break;
        case 1: // Left
            rotate_clockwise_adjacent_faces_left();
            break;
        case 2: // Front
            rotate_clockwise_adjacent_faces_front();
            break;
        case 3: // Right
          //  printf("rotate_clockwise_adjacent_faces_right\n");
            rotate_clockwise_adjacent_faces_right();
            break;
        case 4: // Back
            rotate_clockwise_adjacent_faces_back();
            break;
        case 5: // Down
            rotate_clockwise_adjacent_faces_down();
    }
}


void rotate_clockwise_adjacent_faces_up()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[UP][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[UP][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[UP][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[UP][3]];

    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[0][i]; // Top row of the up face
        right[i] = adj_right[0][i]; // Top row of the right face
        bottom[i] = adj_bottom[0][i]; // Top row of the down face
        left[i] = adj_left[0][i]; // Top row of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[0][i] = left[i]; // Top row of the top face
        adj_left[0][i] = bottom[i]; // Top row of the left face
        adj_right[0][i] = top[i]; // Top row of the right face
        adj_bottom[0][i] = right[i]; // Top row of the bottom face
    }
}

void rotate_clockwise_adjacent_faces_right()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[RIGHT][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[RIGHT][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[RIGHT][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[RIGHT][3]];
  //  printf("\nadjcaent\n%d%d%d%d\n\n",adjacent[RIGHT][0],adjacent[RIGHT][1],adjacent[RIGHT][2],adjacent[RIGHT][3]);
    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[i][CUBE_SIZE-1]; // Right col of the up face
        right[i] = adj_right[i][0]; // Left col of the right face
        bottom[i] = adj_bottom[i][CUBE_SIZE-1]; // Right col of the down face
        left[i] = adj_left[i][CUBE_SIZE-1]; // Right col of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[i][CUBE_SIZE-1] = left[i]; 
        adj_right[CUBE_SIZE-i-1][0] = top[i]; 
        adj_bottom[CUBE_SIZE-i-1][CUBE_SIZE-1] = right[i]; 
        adj_left[i][CUBE_SIZE-1] = bottom[i]; 
    }
}

void rotate_clockwise_adjacent_faces_front()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[FRONT][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[FRONT][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[FRONT][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[FRONT][3]];

    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[CUBE_SIZE - 1][i]; // Bottom row of the top face
        right[i] = adj_right[i][0]; // Left column of the right face
        bottom[i] = adj_bottom[0][i]; // Top row of the bottom face
        left[i] = adj_left[CUBE_SIZE - 1 - i][CUBE_SIZE - 1]; // Right column of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[CUBE_SIZE - 1][i] = left[i]; // Bottom row of the top face
        adj_left[i][CUBE_SIZE - 1] = bottom[i]; // Right column of the left face
        adj_right[i][0] = top[i]; // Left column of the right face
        adj_bottom[0][CUBE_SIZE-1-i] = right[i]; // Top row of the bottom face
    }
}

void rotate_clockwise_adjacent_faces_back()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[BACK][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[BACK][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[BACK][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[BACK][3]];
  //  printf("\nadjcaent\n%d%d%d%d\n\n",adjacent[RIGHT][0],adjacent[RIGHT][1],adjacent[RIGHT][2],adjacent[RIGHT][3]);
    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[0][i]; // Top row of up face
        right[i] = adj_right[i][0]; // Left col of the right face
        bottom[i] = adj_bottom[CUBE_SIZE-1][i]; // Bottom Row of the down face
        left[i] = adj_left[i][CUBE_SIZE-1]; // Right col of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[0][i] = left[i]; 
        adj_right[CUBE_SIZE-i-1][0] = top[i]; 
        adj_bottom[CUBE_SIZE-1][i] = right[i]; 
        adj_left[CUBE_SIZE-i-1][CUBE_SIZE-1] = bottom[i]; 
    }
}
void rotate_clockwise_adjacent_faces_left()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[LEFT][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[LEFT][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[LEFT][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[LEFT][3]];
  //  printf("\nadjcaent\n%d%d%d%d\n\n",adjacent[RIGHT][0],adjacent[RIGHT][1],adjacent[RIGHT][2],adjacent[RIGHT][3]);
    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[i][0]; // Left col of up face
        right[i] = adj_right[i][0]; // Left col of the right face
        bottom[i] = adj_bottom[i][0]; // Left col of the down face
        left[i] = adj_left[i][CUBE_SIZE-1]; // Right col of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[CUBE_SIZE-i-1][0] = left[i]; 
        adj_right[i][0] = top[i]; 
        adj_bottom[i][0] = right[i]; 
        adj_left[CUBE_SIZE-i-1][CUBE_SIZE-1] = bottom[i]; 
    }
}
void rotate_clockwise_adjacent_faces_down()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[DOWN][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[DOWN][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[DOWN][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[DOWN][3]];
  //  printf("\nadjcaent\n%d%d%d%d\n\n",adjacent[RIGHT][0],adjacent[RIGHT][1],adjacent[RIGHT][2],adjacent[RIGHT][3]);
    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[CUBE_SIZE-1][i]; // Bottom row of up face
        right[i] = adj_right[CUBE_SIZE-1][i]; // Bottom row of the right face
        bottom[i] = adj_bottom[CUBE_SIZE-1][i]; // Bottom row of the down face
        left[i] = adj_left[CUBE_SIZE-1][i]; // Bottom row of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[CUBE_SIZE-1][i] = left[i]; 
        adj_right[CUBE_SIZE-1][i] = top[i]; 
        adj_bottom[CUBE_SIZE-1][i] = right[i]; 
        adj_left[CUBE_SIZE-1][i] = bottom[i]; 
    };
}

void rotate_anticlockwise(int face_index)
{
    int (*face)[CUBE_SIZE] = cube[face_index];

    int temp[CUBE_SIZE][CUBE_SIZE];

    // Copy the face to a temporary array
    for (int i = 0; i < CUBE_SIZE; i++) {

        for (int j = 0; j < CUBE_SIZE; j++) {
            temp[i][j] = face[i][j];
        }
    }
    // Rotate the face 90 degrees anti clockwise
    for (int i = 0; i < CUBE_SIZE; i++) {
        for (int j = 0; j < CUBE_SIZE; j++) {
            face[CUBE_SIZE - 1 - j][i] = temp[i][j];
        }
    }

    switch(face_index) {
        case 0: // Up
            rotate_anticlockwise_adjacent_faces_up();
            break;
        case 1: // Left
            rotate_anticlockwise_adjacent_faces_left();
            break;
        case 2: // Front
            rotate_anticlockwise_adjacent_faces_front();
            break;
        case 3: // Right
          //  printf("rotate_clockwise_adjacent_faces_right\n");
            rotate_anticlockwise_adjacent_faces_right();
            break;
        case 4: // Back
            rotate_anticlockwise_adjacent_faces_back();
            break;
        case 5: // Down
            rotate_anticlockwise_adjacent_faces_down();
    }
}

void rotate_anticlockwise_adjacent_faces_up()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[UP][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[UP][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[UP][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[UP][3]];

    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[0][i]; // Top row of the up face
        right[i] = adj_right[0][i]; // Top row of the right face
        bottom[i] = adj_bottom[0][i]; // Top row of the down face
        left[i] = adj_left[0][i]; // Top row of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[0][i] = right[i]; // Top row of the top face
        adj_left[0][i] = top[i]; // Top row of the left face
        adj_right[0][i] = bottom[i]; // Top row of the right face
        adj_bottom[0][i] = left[i]; // Top row of the bottom face
    }
}

void rotate_anticlockwise_adjacent_faces_right()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[RIGHT][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[RIGHT][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[RIGHT][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[RIGHT][3]];
  //  printf("\nadjcaent\n%d%d%d%d\n\n",adjacent[RIGHT][0],adjacent[RIGHT][1],adjacent[RIGHT][2],adjacent[RIGHT][3]);
    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[i][CUBE_SIZE-1]; // Right col of the up face
        right[i] = adj_right[i][0]; // Left col of the right face
        bottom[i] = adj_bottom[i][CUBE_SIZE-1]; // Right col of the down face
        left[i] = adj_left[i][CUBE_SIZE-1]; // Right col of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[i][CUBE_SIZE-1] = right[i]; 
        adj_right[CUBE_SIZE-i-1][0] = bottom[i]; 
        adj_bottom[CUBE_SIZE-i-1][CUBE_SIZE-1] = left[i]; 
        adj_left[i][CUBE_SIZE-1] = top[i]; 
    }
}

void rotate_anticlockwise_adjacent_faces_front()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[FRONT][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[FRONT][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[FRONT][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[FRONT][3]];

    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[CUBE_SIZE - 1][i]; // Bottom row of the top face
        right[i] = adj_right[i][0]; // Left column of the right face
        bottom[i] = adj_bottom[0][i]; // Top row of the bottom face
        left[i] = adj_left[CUBE_SIZE - 1 - i][CUBE_SIZE - 1]; // Right column of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[CUBE_SIZE - 1][i] = right[i]; // Bottom row of the top face
        adj_left[i][CUBE_SIZE - 1] = top[i]; // Right column of the left face
        adj_right[i][0] = bottom[i]; // Left column of the right face
        adj_bottom[0][CUBE_SIZE-1-i] = left[i]; // Top row of the bottom face
    }
}

void rotate_anticlockwise_adjacent_faces_back()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[BACK][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[BACK][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[BACK][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[BACK][3]];
  //  printf("\nadjcaent\n%d%d%d%d\n\n",adjacent[RIGHT][0],adjacent[RIGHT][1],adjacent[RIGHT][2],adjacent[RIGHT][3]);
    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[0][i]; // Top row of up face
        right[i] = adj_right[i][0]; // Left col of the right face
        bottom[i] = adj_bottom[CUBE_SIZE-1][i]; // Bottom Row of the down face
        left[i] = adj_left[i][CUBE_SIZE-1]; // Right col of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[0][i] = right[i]; 
        adj_right[CUBE_SIZE-i-1][0] = bottom[i]; 
        adj_bottom[CUBE_SIZE-1][i] = left[i]; 
        adj_left[CUBE_SIZE-i-1][CUBE_SIZE-1] = top[i]; 
    }
}

void rotate_anticlockwise_adjacent_faces_left()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[LEFT][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[LEFT][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[LEFT][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[LEFT][3]];
  //  printf("\nadjcaent\n%d%d%d%d\n\n",adjacent[RIGHT][0],adjacent[RIGHT][1],adjacent[RIGHT][2],adjacent[RIGHT][3]);
    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[i][0]; // Left col of up face
        right[i] = adj_right[i][0]; // Left col of the right face
        bottom[i] = adj_bottom[i][0]; // Left col of the down face
        left[i] = adj_left[i][CUBE_SIZE-1]; // Right col of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[CUBE_SIZE-i-1][0] = right[i]; 
        adj_right[i][0] = bottom[i]; 
        adj_bottom[i][0] = left[i]; 
        adj_left[CUBE_SIZE-i-1][CUBE_SIZE-1] = top[i]; 
    }
}

void rotate_anticlockwise_adjacent_faces_down()
{
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[DOWN][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[DOWN][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[DOWN][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[DOWN][3]];
  //  printf("\nadjcaent\n%d%d%d%d\n\n",adjacent[RIGHT][0],adjacent[RIGHT][1],adjacent[RIGHT][2],adjacent[RIGHT][3]);
    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];
    
    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[CUBE_SIZE-1][i]; // Bottom row of up face
        right[i] = adj_right[CUBE_SIZE-1][i]; // Bottom row of the right face
        bottom[i] = adj_bottom[CUBE_SIZE-1][i]; // Bottom row of the down face
        left[i] = adj_left[CUBE_SIZE-1][i]; // Bottom row of the left face
    }
    
    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[CUBE_SIZE-1][i] = right[i]; 
        adj_right[CUBE_SIZE-1][i] = bottom[i]; 
        adj_bottom[CUBE_SIZE-1][i] = left[i]; 
        adj_left[CUBE_SIZE-1][i] = top[i]; 
    };
}

void print_faces() {
    for (int k = 0; k < FACES; k++) {
        printf("FACE %d:\n", k);
        for (int i = 0; i < CUBE_SIZE; i++) {
            for (int j = 0; j < CUBE_SIZE; j++) {
                printf("%d ", cube[k][i][j]);
            }
            printf("\n");
        }
        printf("\n");
    }
}
void print_cube_faces() {
    for (int k = 0; k < FACES; k++) {
        //printf("FACE %d:\n", k);
        for (int i = 0; i < CUBE_SIZE; i++) {
            for (int j = 0; j < CUBE_SIZE; j++) {
                printf("%d", cube[k][i][j]);
            }
            printf("\n");
        }
        printf("\n");
    }
    printf("\n");
}

void print_cube() {
    for (int k = 0; k < FACES; k++) {
        //printf("FACE %d:\n", k);
        for (int i = 0; i < CUBE_SIZE; i++) {
            for (int j = 0; j < CUBE_SIZE; j++) {
                printf("%d", cube[k][i][j]);
            }
     //       printf("\n");
        }
   //     printf("\n");
    }
   printf("\n");
}
void get_cube(int cur_cube[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE])
{
    for (int k = 0; k < FACES; k++) {
        for (int i = 0; i < CUBE_SIZE; i++) {
            for (int j = 0; j < CUBE_SIZE; j++) {
                cur_cube[k][i][j] =cube[k][i][j];
            }
        }
    }
}

void init_cube(int cur_cube[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE])
{
    for (int k = 0; k < FACES; k++) {
        for (int i = 0; i < CUBE_SIZE; i++) {
            for (int j = 0; j < CUBE_SIZE; j++) {
                cube[k][i][j] = cur_cube[k][i][j];
            }
        }
    }
}

bool is_solved()
{
    for (int k = 0; k < FACES; k++) {
        for (int i = 0; i < CUBE_SIZE; i++) {
            for (int j = 0; j < CUBE_SIZE; j++) {
                if(cube[k][i][j] != solved_cube[k][i][j])
                    return false;
            }
        }
    }
    return true;
}