#pragma once

#define BOARD_SIZE 4
#define GOAL 3
#define ALLOW_EXCEED 1
#define N_GRIDS (BOARD_SIZE * BOARD_SIZE)
#define GET_INDEX(i, j) ((i) * (BOARD_SIZE) + (j))
#define GET_COL(x) ((x) % BOARD_SIZE)
#define GET_ROW(x) ((x) / BOARD_SIZE)

#define for_each_empty_grid(i, table) \
    for (int i = 0; i < N_GRIDS; i++) \
        if (table[i] == ' ')

typedef struct {
    int i_shift, j_shift;
    int i_lower_bound, j_lower_bound, i_upper_bound, j_upper_bound;
} line_t;

// Define structure based on board size
#if N_GRIDS <= 64  // Maximum 8x8 board
typedef struct {
    uint8_t reset : 1;
    uint8_t player : 1;    // 0 for 'X', 1 for 'O'
    uint8_t position : 6;  // 6 bits sufficient for 0-63
} __attribute__((packed)) move_event_t;
#elif N_GRIDS <= 256  // Maximum 16x16 board
typedef struct {
    uint8_t reset : 1;
    uint8_t player : 1;     // 0 for 'X', 1 for 'O'
    uint16_t position : 8;  // 8 bits sufficient for 0-255
} __attribute__((packed)) move_event_t;
#else                 // Larger boards
typedef struct {
    uint8_t reset : 1;
    uint8_t player : 1;  // 0 for 'X', 1 for 'O'
    uint16_t position;   // Full 16 bits
} __attribute__((packed)) move_event_t;
#endif


/* Self-defined fixed-point type, using last 10 bits as fractional bits,
 * starting from lsb */
#define FIXED_SCALE_BITS 8
#define FIXED_MAX (~0U)
#define FIXED_MIN (0U)
#define GET_SIGN(x) ((x) & (1U << 31))
#define SET_SIGN(x) ((x) | (1U << 31))
#define CLR_SIGN(x) ((x) & ((1U << 31) - 1U))
typedef unsigned fixed_point_t;

#define DRAW_SIZE (N_GRIDS + BOARD_SIZE)
#define DRAWBUFFER_SIZE                                                 \
    ((BOARD_SIZE * (BOARD_SIZE + 1) << 1) + (BOARD_SIZE * BOARD_SIZE) + \
     ((BOARD_SIZE << 1) + 1) + 1)

extern const line_t lines[4];

int *available_moves(const char *table);
char check_win(const char *t);
fixed_point_t calculate_win_value(char win, char player);
