#ifndef RUBIKS_H
#define RUBIKS_H

char** split_string(char *str, int *count);
bool is_valid_move(char move);
bool validate_moves(char *moves);

#endif // RUBIKS_H