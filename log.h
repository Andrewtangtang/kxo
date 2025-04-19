
#include <stddef.h>
#include <stdint.h>
#include "game.h"
#define MAX_MOVES (BOARD_SIZE * BOARD_SIZE)

typedef struct {
    uint8_t moves[MAX_MOVES / 2];  // each byte stores two moves (4 bits)
    int move_count;                // current move count
} game_record_t;

typedef struct {
    game_record_t *data;  // pointer to records
    size_t size;          // number of used records
    size_t cap;           // array capacity
} game_record_list_t;

void init_game_list(game_record_list_t *list);
void free_game_list(game_record_list_t *list);
int ensure_capacity(game_record_list_t *list, size_t new_cap);
int add_new_game(game_record_list_t *list);
void set_move(game_record_list_t *list, int game_idx, int step, uint8_t pos);
uint8_t get_move(const game_record_list_t *list, int game_idx, int step);
void print_game_list(const game_record_list_t *list);



