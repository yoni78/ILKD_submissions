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

int cube[FACES][CUBE_SIZE][CUBE_SIZE];

int F_orig_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
    {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {16, 17, 18}
    },
    {
        {19, 20, 21}, 
        {22, 23, 24}, 
        {25, 26, 27}
    },
    {
        {28, 29, 30}, 
        {31, 32, 33}, 
        {34, 35, 36}
    },
    {
        {37, 38, 39},
        {40, 41, 42}, 
        {43, 44, 45}
    },
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    },
    {
        {55, 56, 57}, 
        {58, 59, 60}, 
        {61, 62, 63}
    }
};

int F_result_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
   {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {27, 24, 21}
    },
    {
        {19, 20, 55}, 
        {22, 23, 56}, 
        {25, 26, 57}
    },
    {
        {34, 31, 28}, 
        {35, 32, 29}, 
        {36, 33, 30}
    },
    {
        {16, 38, 39},
        {17, 41, 42}, 
        {18, 44, 45}
    },
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    },
    {
        {43, 40, 37}, 
        {58, 59, 60}, 
        {61, 62, 63}
    }
};

int U_orig_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
    {
        {28, 29, 30}, 
        {31, 32, 33}, 
        {34, 35, 36}
    },
    {
        {19, 20, 21}, 
        {22, 23, 24}, 
        {25, 26, 27}
    },
    {
        {55, 56, 57}, 
        {58, 59, 60}, 
        {61, 62, 63}
    },
    {
        {37, 38, 39},
        {40, 41, 42}, 
        {43, 44, 45}
    },
    {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {16, 17, 18}
    },
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    }
};

int U_result_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
    {
        {34, 31, 28}, 
        {35, 32, 29}, 
        {36, 33, 30}
    },
    {
        {19, 20, 55}, 
        {22, 23, 56}, 
        {25, 26, 57}
    },
    {
        {43, 40, 37}, 
        {58, 59, 60}, 
        {61, 62, 63}
    },
    {
        {16, 38, 39},
        {17, 41, 42}, 
        {18, 44, 45}
    },
    {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {27, 24, 21}
    },
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    }
};


int R_orig_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
    {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {16, 17, 18}
    },
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    },
    {
        {19, 20, 21}, 
        {22, 23, 24}, 
        {25, 26, 27}
    },
    {
        {28, 29, 30}, 
        {31, 32, 33}, 
        {34, 35, 36}
    },
    {
        {37, 38, 39},
        {40, 41, 42}, 
        {43, 44, 45}
    },
    {
        {55, 56, 57}, 
        {58, 59, 60}, 
        {61, 62, 63}
    }
};

int R_result_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
   {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {27, 24, 21}
    },
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    },
    {
        {19, 20, 55}, 
        {22, 23, 56}, 
        {25, 26, 57}
    },
    {
        {34, 31, 28}, 
        {35, 32, 29}, 
        {36, 33, 30}
    },
    {
        {16, 38, 39},
        {17, 41, 42}, 
        {18, 44, 45}
    },
    {
        {43, 40, 37}, 
        {58, 59, 60}, 
        {61, 62, 63}
    }
};

int L_orig_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
    {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {16, 17, 18}
    },
    {
        {28, 29, 30}, 
        {31, 32, 33}, 
        {34, 35, 36}
    },
    {
        {37, 38, 39},
        {40, 41, 42}, 
        {43, 44, 45}
    },
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    },
    {
        {19, 20, 21}, 
        {22, 23, 24}, 
        {25, 26, 27}
    },
    {
        {55, 56, 57}, 
        {58, 59, 60}, 
        {61, 62, 63}
    }
};

int L_result_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
   {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {27, 24, 21}
    },
    {
        {34, 31, 28}, 
        {35, 32, 29}, 
        {36, 33, 30}
    },
    {
        {16, 38, 39},
        {17, 41, 42}, 
        {18, 44, 45}
    },
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    },
    {
        {19, 20, 55}, 
        {22, 23, 56}, 
        {25, 26, 57}
    },
    {
        {43, 40, 37}, 
        {58, 59, 60}, 
        {61, 62, 63}
    }
};

int B_orig_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
    {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {16, 17, 18}
    },
    {
        {37, 38, 39},
        {40, 41, 42}, 
        {43, 44, 45}
    },
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    },
    {
        {19, 20, 21}, 
        {22, 23, 24}, 
        {25, 26, 27}
    },
    {
        {28, 29, 30}, 
        {31, 32, 33}, 
        {34, 35, 36}
    },
    {
        {55, 56, 57}, 
        {58, 59, 60}, 
        {61, 62, 63}
    }
};

int B_result_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
   {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {27, 24, 21}
    },
    {
        {16, 38, 39},
        {17, 41, 42}, 
        {18, 44, 45}
    },
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    },
    {
        {19, 20, 55}, 
        {22, 23, 56}, 
        {25, 26, 57}
    },
    {
        {34, 31, 28}, 
        {35, 32, 29}, 
        {36, 33, 30}
    },
    {
        {43, 40, 37}, 
        {58, 59, 60}, 
        {61, 62, 63}
    }
};

int D_orig_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
    {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    },
    {
        {19, 20, 21}, 
        {22, 23, 24}, 
        {25, 26, 27}
    },
    {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {16, 17, 18}
    },
    {
        {37, 38, 39},
        {40, 41, 42}, 
        {43, 44, 45}
    },
    {
        {55, 56, 57}, 
        {58, 59, 60}, 
        {61, 62, 63}
    },
    {
        {28, 29, 30}, 
        {31, 32, 33}, 
        {34, 35, 36}
    }
};

int D_result_cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
   {
        {46, 47, 48}, 
        {49, 50, 51}, 
        {52, 53, 54}
    },
    {
        {19, 20, 55}, 
        {22, 23, 56}, 
        {25, 26, 57}
    },
    {
        {10, 11, 12}, 
        {13, 14, 15}, 
        {27, 24, 21}
    },
    {
        {16, 38, 39},
        {17, 41, 42}, 
        {18, 44, 45}
    },
    {
        {43, 40, 37}, 
        {58, 59, 60}, 
        {61, 62, 63}
    },
    {
        {34, 31, 28}, 
        {35, 32, 29}, 
        {36, 33, 30}
    }
};

bool is_identical(int cube1[FACES][CUBE_SIZE][CUBE_SIZE], int cube2[FACES][CUBE_SIZE][CUBE_SIZE])
{
    for (int k = 0; k < FACES; k++) {
        for (int i = 0; i < CUBE_SIZE; i++) {
            for (int j = 0; j < CUBE_SIZE; j++) {
                if(cube1[k][i][j] != cube2[k][i][j])
                {
                    printf("mismatch: face %d: (%d,%d)\n",k,i,j);
                    printf("values are %d and %d\n",cube1[k][i][j], cube2[k][i][j]);
                    return false;
                }
            }   
        }
    }
    return true;
}

bool split_string_test() {
    char *s = " abc  def 123  ";

    int tokens_count = 0;
    char **res = split_string(s, &tokens_count);

    if (tokens_count != 3) {
        free_tokens(res, tokens_count);
        return false;
    }

    if (strcmp(res[0], "abc") == 1) {
        free_tokens(res, tokens_count);
        return false;
    }

    if (strcmp(res[1], "def") == 1) {
        free_tokens(res, tokens_count);
        return false;
    }

    if (strcmp(res[2], "123") == 1) {
        free_tokens(res, tokens_count);
        return false;
    }
    
    free_tokens(res, tokens_count);
    return true;
}

bool test_validate_moves() {
    if (!validate_moves("  F R   U  L L B D")) {
        return false;
    }

    if (!validate_moves("F'  F R   U'  L L B D F' F' R' U' L' L' B' D'")) {
        return false;
    }

    if (validate_moves("F R A")) {
        return false;
    }

    if (validate_moves("f")) {
        return false;
    }

    if (validate_moves("FF")) {
        return false;
    }

    if (validate_moves("D!")) {
        return false;
    }
    
    if (validate_moves("FRR")) {
        return false;
    }

    return true;
}

//F, R, U, L, B, D

bool test_F() {
    init_cube(F_orig_cube);
    process_moves("F");
    get_cube(cube);
    return is_identical(F_result_cube, cube);
}
bool test_U() {
    init_cube(U_orig_cube);
    process_moves("U");
    get_cube(cube);
    return is_identical(U_result_cube, cube);
}
bool test_R() {
    init_cube(R_orig_cube);
    process_moves("R");
    get_cube(cube);
    return is_identical(R_result_cube, cube);
}
bool test_L() {
    init_cube(L_orig_cube);
    process_moves("L");
    get_cube(cube);
    return is_identical(L_result_cube, cube);
}
bool test_B() {
    init_cube(B_orig_cube);
    process_moves("B");
    get_cube(cube);
    return is_identical(B_result_cube, cube);
}
bool test_D() {
    init_cube(D_orig_cube);
    process_moves("D");
    get_cube(cube);
    return is_identical(D_result_cube, cube);
}
//anti clockwise
bool test_F_tag() {
    init_cube(F_orig_cube);
    process_moves("F F'");
    get_cube(cube);
    return is_identical(F_orig_cube, cube);
}
bool test_U_tag() {
    init_cube(U_orig_cube);
    process_moves("U U'");
    get_cube(cube);
    return is_identical(U_orig_cube, cube);
}
bool test_R_tag() {
    init_cube(R_orig_cube);
    process_moves("R R'");
    get_cube(cube);
    return is_identical(R_orig_cube, cube);
}
bool test_L_tag() {
    init_cube(L_orig_cube);
    process_moves("L L'");
    get_cube(cube);
    return is_identical(L_orig_cube, cube);
}
bool test_B_tag() {
    init_cube(B_orig_cube);
    process_moves("B B'");
    get_cube(cube);
    return is_identical(B_orig_cube, cube);
}
bool test_D_tag() {
    init_cube(D_orig_cube);
    process_moves("D D'");
    get_cube(cube);
    return is_identical(D_orig_cube, cube);
}

int main() {
    
    RUN_TEST(split_string_test);
    RUN_TEST(test_validate_moves);
    RUN_TEST(test_F);
    RUN_TEST(test_U);
    RUN_TEST(test_R);
    RUN_TEST(test_L);
    RUN_TEST(test_B);
    RUN_TEST(test_D);
    RUN_TEST(test_F_tag);
    RUN_TEST(test_U_tag);
    RUN_TEST(test_R_tag);
    RUN_TEST(test_L_tag);
    RUN_TEST(test_B_tag);
    RUN_TEST(test_D_tag);
}