#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/random.h>

#define DEVICE_NAME "cube"

#define FACES 6
#define FACE_PIECES 9

static int major_number;
static char cube[6][9];
static DEFINE_MUTEX(cube_mutex);

#define CUBE_SETUP _IOW('a', 1, unsigned short)
#define CUBE_IS_SOLVED _IOR('a', 2, unsigned short)

int process_moves(char *moves);

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

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    mutex_lock(&cube_mutex);
    // TODO: Implement read functionality for cube state
    // TODO: Return -1 and set errno on error
    mutex_unlock(&cube_mutex);

    return 0;
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
    kfree(kbuf);
exit:
    mutex_unlock(&cube_mutex);

    return res;
}

static void perform_random_moves(unsigned short num_of_moves) {
    char *possible_moves[] = {"F", "L", "U", "R", "B", "D","F'", "L'", "U'", "R'", "B'", "D'"};

    for (int i = 0; i < num_of_moves; i++) {
        int move_index = get_random_int() % 12;

        process_moves(possible_moves[move_index]);
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
            if (CUBE_SIZE + offset < 0) {
                new_pos = -EINVAL;
                goto exit;
            }

            new_pos = CUBE_SIZE + offset;
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
