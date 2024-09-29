#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/string.h>
#include <linux/errname.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

#define DEVICE_NAME "cube"
#define CLASS_NAME "cube"

#define FACES 6
#define FACE_PIECES 9
#define CUBE_SIZE 3
#define CUBE_PIECES FACES * FACE_PIECES

#define UP 0 
#define LEFT 1
#define FRONT 2
#define RIGHT 3
#define BACK 4
#define DOWN 5

static dev_t major_number;
static struct cdev cube_dev;
static struct class *class;
static DEFINE_MUTEX(cube_mutex);

#define CUBE_SETUP _IOW('c', 1, unsigned short)
#define CUBE_IS_SOLVED _IOR('c', 2, unsigned short)

static int dev_open(struct inode *inodep, struct file *filep);
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset);
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);
static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
static loff_t dev_lseek(struct file *filep, loff_t offset, int whence);
static int dev_release(struct inode *inodep, struct file *filep);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
    .release = dev_release,
    .llseek = dev_lseek
};

static int cube[FACES][CUBE_SIZE][CUBE_SIZE] = {
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

static void free_tokens(char **tokens, int tokens_count) {
    for (int i = 0; i < tokens_count; i++) {
        kfree(tokens[i]);
    }

    kfree(tokens);
}

static char** split_string(char *str, int *count) {
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

    char** tokens = kmalloc(tokens_count * sizeof(char*), GFP_KERNEL);
    int token_index = 0;
    int start = -1;
    int length = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == ' ') {
            if (start != -1) {
                tokens[token_index] = kmalloc((length + 1) * sizeof(char), GFP_KERNEL);
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
        tokens[token_index] = kmalloc((length + 1) * sizeof(char), GFP_KERNEL);
        strncpy(tokens[token_index], &str[start], length);
        tokens[token_index][length] = '\0';
    }

    *count = tokens_count;

    return tokens;
}

static bool is_valid_move(char move) {
    return move == 'F' || move == 'R' || move == 'U' || move == 'L' || move == 'B' || move == 'D';
}

static bool validate_moves(char *moves) {
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

static int get_face_index(char c)
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

void rotate_clockwise_adjacent_faces_front(void);
void rotate_clockwise_adjacent_faces_up(void);
void rotate_clockwise_adjacent_faces_right(void);
void rotate_clockwise_adjacent_faces_back(void);
void rotate_clockwise_adjacent_faces_left(void);
void rotate_clockwise_adjacent_faces_down(void);

void rotate_anticlockwise_adjacent_faces_front(void);
void rotate_anticlockwise_adjacent_faces_up(void);
void rotate_anticlockwise_adjacent_faces_right(void);
void rotate_anticlockwise_adjacent_faces_back(void);
void rotate_anticlockwise_adjacent_faces_left(void);
void rotate_anticlockwise_adjacent_faces_down(void);

static void exec_move(char* move)
{
    size_t token_size = strlen(move);
    int face_index = get_face_index(move[0]);

    if(token_size == 1)
        rotate_clockwise(face_index);
    else if (token_size == 2)
        rotate_anticlockwise(face_index);
}

static int process_moves(char *moves) {
    if (!validate_moves(moves)) {
        return -1;
    }

    int tokens_count = 0;
    char **tokens = split_string(moves, &tokens_count);

    for(int i = 0; i < tokens_count; i++)
    {
        exec_move(tokens[i]);
    }

    free_tokens(tokens, tokens_count);

    return tokens_count;
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
        adj_top[CUBE_SIZE-i-1][CUBE_SIZE-1] = right[i]; 
        adj_right[CUBE_SIZE-i-1][0] = bottom[i]; 
        adj_bottom[i][CUBE_SIZE-1] = left[i]; 
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
        adj_left[CUBE_SIZE-i-1][CUBE_SIZE - 1] = top[i]; // Right column of the left face
        adj_right[CUBE_SIZE-i-1][0] = bottom[i]; // Left column of the right face
        adj_bottom[0][i] = left[i]; // Top row of the bottom face
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
        adj_top[0][CUBE_SIZE-i-1] = right[i]; 
        adj_right[i][0] = bottom[i]; 
        adj_bottom[CUBE_SIZE-1][CUBE_SIZE-i-1] = left[i]; 
        adj_left[i][CUBE_SIZE-1] = top[i]; 
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
        adj_top[i][0] = right[i]; 
        adj_right[i][0] = bottom[i]; 
        adj_bottom[CUBE_SIZE-i-1][0] = left[i]; 
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

static char * devnode(const struct device *dev, umode_t * mode)
{
    if (mode) {
        *mode = 0644;
    }

    return NULL;
}

static int __init cube_init(void) {
    int ret = 0;

    if ((ret = alloc_chrdev_region(&major_number, 0, 1, DEVICE_NAME))) {
        pr_err("Unable to allocate cube device: %s\n", errname(ret));
        goto err_alloc_chrdev_region;
    }

    class = class_create(CLASS_NAME);

    if (IS_ERR(class)) {
        ret = PTR_ERR(class);
        pr_err("Failed to create device class: %s\n", errname(ret));
        goto err_class_create;
    }

    class->devnode = devnode;

    cdev_init(&cube_dev, &fops);

    if ((ret = cdev_add(&cube_dev, major_number, 1))) {
        pr_err("Failed to add cdev: %s\n", errname(ret));
        goto err_cdev_add;
    }

    struct device *dev = device_create(class, NULL, major_number, NULL, DEVICE_NAME);

    if (IS_ERR(dev)) {
        ret = PTR_ERR(dev);
        pr_err("Failed to create device: %s\n", errname(ret));
        goto err_device_create;
    }

    mutex_init(&cube_mutex);
    pr_info("Cube device initialized\n");

    return 0;

err_device_create:
    device_destroy(class, major_number);

err_cdev_add:
    class_destroy(class);

err_class_create:
    unregister_chrdev_region(major_number, 1);

err_alloc_chrdev_region:
    return ret;
}

static void __exit cube_exit(void) {
    mutex_destroy(&cube_mutex);
    device_destroy(class, major_number);
    class_destroy(class);
    unregister_chrdev_region(major_number, 1);
    cdev_del(&cube_dev);

    pr_info("Cube device exited\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    pr_info("Cube device opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    pr_info("Cube device closed\n");
    return 0;
}

static char *convert_cube_to_string(void) {
    char *cube_str = kmalloc(CUBE_PIECES + 1, GFP_KERNEL);

    if (!cube_str) {
        return NULL;
    }

    int index = 0;

    for (int face = 0; face < FACES; face++) {
        for (int row = 0; row < CUBE_SIZE; row++) {
            for (int col = 0; col < CUBE_SIZE; col++) {
                cube_str[index++] = '0' + cube[face][row][col];
            }
        }
    }

    cube_str[CUBE_PIECES] = '\0';

    return cube_str;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    if (len == 0 || *offset >= CUBE_PIECES) {
        return 0;
    }

    mutex_lock(&cube_mutex);

    ssize_t bytes_read = 0;

    if (*offset + len > CUBE_PIECES) {
        len = CUBE_PIECES - *offset;
    }

    char *cube_str = convert_cube_to_string();

    if (!cube_str) {
        bytes_read = -ENOMEM;
        goto exit;
    }

    if (copy_to_user(buffer, cube_str + *offset, len)) {
        bytes_read = -EFAULT;
        goto free_and_exit;
    }

    *offset += len;
    bytes_read = len;

free_and_exit:
    kfree(cube_str);

exit:
    mutex_unlock(&cube_mutex);

    return bytes_read;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    ssize_t res = 0;

    mutex_lock(&cube_mutex);
    char *moves = kmalloc(len + 1, GFP_KERNEL);

    if (!moves) {
        res = -ENOMEM;
        goto exit;
    }

    if (copy_from_user(moves, buffer, len)) {
        res = -EINVAL; // TODO: This means that the buffer is invalid?
        goto free_and_exit;
    }

    moves[len] = '\0';

    int num_of_moves = process_moves(moves);

    if (num_of_moves == -1) {
        res = -EPERM;
        goto free_and_exit;
    }

    res = num_of_moves;

free_and_exit:
    kfree(moves);

exit:
    mutex_unlock(&cube_mutex);

    return res;
}

static void perform_random_moves(unsigned short num_of_moves) {
    char *possible_moves[] = {"F", "L", "U", "R", "B", "D","F'", "L'", "U'", "R'", "B'", "D'"};

    for (int i = 0; i < num_of_moves; i++) {
        int move_index = get_random_u8() % 12;

        exec_move(possible_moves[move_index]);
    }
}

static void cube_setup(unsigned short num_of_moves) {
    for (int i = 0; i < FACES; i++) {
        for (int j = 0; j < CUBE_SIZE; j++) {
            for (int k = 0; k < CUBE_SIZE; k++) {
                cube[i][j][k] = i;
            }
        }
    }

    perform_random_moves(num_of_moves);
}

static int is_cube_solved(void) {
    for (int i = 0; i < FACES; i++) {
        int face_color = cube[i][0][0];

        for (int j = 0; j < CUBE_SIZE; j++) {
            for (int k = 0; k < CUBE_SIZE; k++) {
                if (cube[i][j][k] != face_color) {
                    return 0;
                }
            }
        }
    }
    
    return 1;
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    mutex_lock(&cube_mutex);

    long res = 0;
    unsigned short user_val = 0;

    switch (cmd) {
        case CUBE_SETUP:
            if (copy_from_user(&user_val, (unsigned short __user *)arg, sizeof(user_val))) {
                res = -EFAULT;
                goto exit;
            }

            cube_setup(user_val);

            break;

        case CUBE_IS_SOLVED:
            user_val = is_cube_solved();

            if (copy_to_user((unsigned short __user *)arg, &user_val, sizeof(user_val))) {
                res = -EFAULT;
                goto exit;
            }

            break;

        default:
            res = -EINVAL;
    }

exit:
    mutex_unlock(&cube_mutex);

    return res;
}

static loff_t dev_lseek(struct file *filep, loff_t offset, int whence) {
    mutex_lock(&cube_mutex);

    loff_t new_pos = 0;

    switch (whence) {
        case SEEK_SET:
            if (offset < 0) {
                new_pos = -EINVAL;
                goto exit;
            }

            new_pos = offset;
            break;

        case SEEK_CUR:
            if (filep->f_pos + offset < 0) {
                new_pos = -EINVAL;
                goto exit;
            }

            new_pos = filep->f_pos + offset;
            break;

        case SEEK_END:
            if (CUBE_PIECES + offset < 0) {
                new_pos = -EINVAL;
                goto exit;
            }

            new_pos = CUBE_PIECES + offset;
            break;

        default:
            new_pos = -EINVAL;
            goto exit;
    }

    filep->f_pos = new_pos;

exit:
    mutex_unlock(&cube_mutex);

    return new_pos;
}

module_init(cube_init);
module_exit(cube_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jonathan-amal");
MODULE_DESCRIPTION("A Rubik's Cube Character Device Module");
MODULE_VERSION("1.0");
