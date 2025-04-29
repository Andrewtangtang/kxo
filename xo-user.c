#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "game.h"
#include "log.h"

#define XO_STATUS_FILE "/sys/module/kxo/initstate"
#define XO_DEVICE_FILE "/dev/kxo"
#define XO_DEVICE_ATTR_FILE "/sys/class/kxo/kxo/kxo_state"

#define GET_INDEX(i, j) ((i) * (BOARD_SIZE) + (j))

char board[N_GRIDS];
static inline void print_hline(void)
{
    for (int i = 0; i < BOARD_SIZE * 2 - 1; ++i)
        putchar('-');
    putchar('\n');
}


void print_board()
{
    time_t now;
    time(&now);  // get current time
    const struct tm *local_time = localtime(&now);
    char time_str[20];

    // format time as HH:MM:SS
    strftime(time_str, sizeof(time_str), "%H:%M:%S", local_time);
    printf("\033[H\033[J");  // clear screen
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            putchar(board[GET_INDEX(r, c)]);
            if (c != BOARD_SIZE - 1)
                putchar('|');
        }
        putchar('\n');
        if (r != BOARD_SIZE - 1)
            print_hline();
    }
    for (int i = 0; i < BOARD_SIZE * 2 - 1; ++i)
        putchar('=');
    putchar('\n');
    printf("Current Time: %s\n", time_str);
}


static bool status_check(void)
{
    FILE *fp = fopen(XO_STATUS_FILE, "r");
    if (!fp) {
        printf("kxo status : not loaded\n");
        return false;
    }

    char read_buf[20];
    fgets(read_buf, 20, fp);
    read_buf[strcspn(read_buf, "\n")] = 0;
    if (strcmp("live", read_buf)) {
        printf("kxo status : %s\n", read_buf);
        fclose(fp);
        return false;
    }
    fclose(fp);
    return true;
}

static struct termios orig_termios;

static void raw_mode_disable(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void raw_mode_enable(void)
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(raw_mode_disable);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~IXON;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static bool read_attr, end_attr;

static void listen_keyboard_handler(void)
{
    int attr_fd = open(XO_DEVICE_ATTR_FILE, O_RDWR);
    char input;

    if (read(STDIN_FILENO, &input, 1) == 1) {
        char buf[20];
        switch (input) {
        case 16: /* Ctrl-P */
            read(attr_fd, buf, 6);
            buf[0] = (buf[0] - '0') ? '0' : '1';
            read_attr ^= 1;
            write(attr_fd, buf, 6);
            if (!read_attr)
                printf("\n\nStopping to display the chess board...\n");
            break;
        case 17: /* Ctrl-Q */
            read(attr_fd, buf, 6);
            buf[4] = '1';
            read_attr = false;
            end_attr = true;
            write(attr_fd, buf, 6);
            printf("\n\nStopping the kernel space tic-tac-toe game...\n");
            break;
        }
    }
    close(attr_fd);
}

int main(int argc, char *argv[])
{
    if (!status_check())
        exit(1);

    raw_mode_enable();
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // char display_buf[DRAWBUFFER_SIZE];
    move_event_t move_event;
    game_record_list_t game_list;
    init_game_list(&game_list);
    add_new_game(&game_list);

    fd_set readset;
    int device_fd = open(XO_DEVICE_FILE, O_RDONLY);
    int max_fd = device_fd > STDIN_FILENO ? device_fd : STDIN_FILENO;
    read_attr = true;
    end_attr = false;

    memset(board, ' ', N_GRIDS);
    while (!end_attr) {
        FD_ZERO(&readset);
        FD_SET(STDIN_FILENO, &readset);
        FD_SET(device_fd, &readset);

        int result = select(max_fd + 1, &readset, NULL, NULL, NULL);
        if (result < 0) {
            printf("Error with select system call\n");
            exit(1);
        }

        if (FD_ISSET(STDIN_FILENO, &readset)) {
            FD_CLR(STDIN_FILENO, &readset);
            listen_keyboard_handler();
        }
        if (FD_ISSET(device_fd, &readset)) {
            FD_CLR(device_fd, &readset);
            read(device_fd, &move_event, sizeof(move_event));
            if (move_event.reset) {
                memset(board, ' ', N_GRIDS);
                add_new_game(&game_list);
            } else {
                if (move_event.player == 0) {
                    board[move_event.position] = 'X';
                } else {
                    board[move_event.position] = 'O';
                }
                int gi = game_list.size - 1;               // current game index
                int step = game_list.data[gi].move_count;  // current step
                uint8_t p = move_event.position;           // current position
                set_move(&game_list, gi, step, p);
            }
            if (read_attr)
                print_board();
        }
    }

    printf("\n\n\n");
    print_game_list(&game_list);
    free_game_list(&game_list);
    raw_mode_disable();
    fcntl(STDIN_FILENO, F_SETFL, flags);

    close(device_fd);

    return 0;
}
