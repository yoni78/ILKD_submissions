#ifndef RUBIKS_H
#define RUBIKS_H

#define FACES 6
#define FACE_PIECES 9
#define CUBE_SIZE 3

char** split_string(char *str, int *count);
void free_tokens(char **tokens, int tokens_count); 
bool is_valid_move(char move);
bool validate_moves(char *moves);
void process_moves(char *moves);
bool is_solved();

void get_cube( int cur_cube[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE]);
void init_cube(int cur_cube[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE]);
void print_faces();
void print_cube();
void print_cube_faces(); 

#endif // RUBIKS_H