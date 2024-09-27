#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/string.h>

#define DEVICE_NAME "cube"

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

static int major_number;
static char cube[6][9];
static DEFINE_MUTEX(cube_mutex);

#define CUBE_SETUP _IOW('a', 1, unsigned short)
#define CUBE_IS_SOLVED _IOR('a', 2, unsigned short)

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

void free_tokens(char **tokens, int tokens_count) {
    for (int i = 0; i < tokens_count; i++) {
        kfree(tokens[i]);
    }

    kfree(tokens);
}

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

void exec_move(char* move)
{
    size_t token_size = strlen(move);
    int face_index = get_face_index(move[0]);

    if(token_size == 1)
        rotate_clockwise(face_index);
    else if (token_size == 2)
        rotate_anticlockwise(face_index);
}

int process_moves(char *moves) {
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
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[face_index][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[face_index][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[face_index][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[face_index][3]];
    
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

void rotate_anticlockwise(int face_index)
{
    int (*face)[CUBE_SIZE] = cube[face_index];
    int (*adj_top)[CUBE_SIZE] = cube[adjacent[face_index][0]];
    int (*adj_right)[CUBE_SIZE] = cube[adjacent[face_index][1]];
    int (*adj_bottom)[CUBE_SIZE] = cube[adjacent[face_index][2]];
    int (*adj_left)[CUBE_SIZE] = cube[adjacent[face_index][3]];

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
            face[CUBE_SIZE - 1 - j][i] = temp[i][j];
        }
    }

    // Temporary arrays to hold the edges of adjacent faces
    int top[CUBE_SIZE], left[CUBE_SIZE], right[CUBE_SIZE], bottom[CUBE_SIZE];

    // Save the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        top[i] = adj_top[CUBE_SIZE - 1][i]; // Bottom row of the top face
        right[i] = adj_right[i][0]; // Left column of the right face
        bottom[i] = adj_bottom[0][i]; // Top row of the bottom face
        left[i] = adj_left[i][CUBE_SIZE - 1]; // Right column of the left face
    }

    // Update the edges of adjacent faces
    for (int i = 0; i < CUBE_SIZE; i++) {
        adj_top[CUBE_SIZE - 1][i] = right[i]; // Bottom row of the top face
        adj_left[CUBE_SIZE - 1 - i][CUBE_SIZE - 1] = top[i]; // Right column of the left face
        adj_right[i][0] = bottom[CUBE_SIZE - 1 - i]; // Left column of the right face
        adj_bottom[0][i] = left[i]; // Top row of the bottom face
    }
}

static int __init cube_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0) {
        printk(KERN_ALERT "Cube failed to register a major number\n");
        return major_number;
    }

    mutex_init(&cube_mutex);
    printk(KERN_INFO "Cube device initialized\n");
    return 0;
}

static void __exit cube_exit(void) {
    mutex_destroy(&cube_mutex);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Cube device exited\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Cube device opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Cube device closed\n");
    return 0;
}

char *convert_cube_to_string() {
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
    sssize_t res = 0;

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
        int move_index = get_random_int() % 12;

        exec_move(possible_moves[move_index]);
    }
}

static void cube_setup(unsigned short num_of_moves) {
    for (int i = 0; i < FACES; i++) {
        for (int j = 0; j < FACE_PIECES; j++) {
            cube[i][j] = i;
        }
    }

    perform_random_moves(num_of_moves);
}

static int is_cube_solved() {
    for (int i = 0; i < FACES; i++) {
        int face_color = cube[i][0];

        for (int j = 1; j < FACE_PIECES; j++) {
            if (cube[i][j] != face_color) {
                return 0;
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

            res = user_val

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

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
    .release = dev_release,
    .llseek = dev_lseek
};

module_init(cube_init);
module_exit(cube_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jonathan-amal");
MODULE_DESCRIPTION("A Rubik's Cube Character Device Module");
MODULE_VERSION("1.0");
