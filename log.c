#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"


const char *pos_table[16] = {"A1", "B1", "C1", "D1", "A2", "B2", "C2", "D2",
                             "A3", "B3", "C3", "D3", "A4", "B4", "C4", "D4"};

void init_game_list(game_record_list_t *list)
{
    list->data = NULL;
    list->size = 0;
    list->cap = 0;
}

void free_game_list(game_record_list_t *list)
{
    free(list->data);
    list->data = NULL;
    list->size = list->cap = 0;
}

int ensure_capacity(game_record_list_t *lst, size_t new_cap)
{
    if (lst->cap >= new_cap)
        return 0;
    size_t target = lst->cap ? lst->cap * 2 : 4;
    if (target < new_cap)
        target = new_cap;
    game_record_t *p = realloc(lst->data, target * sizeof(*p));
    if (!p)
        return -1;
    lst->data = p;
    lst->cap = target;
    return 0;
}

// add new game then reset move count
int add_new_game(game_record_list_t *lst)
{
    if (ensure_capacity(lst, lst->size + 1) < 0)
        return -1;
    game_record_t *rec = &lst->data[lst->size++];
    memset(rec->moves, 0, sizeof(rec->moves));
    rec->move_count = 0;
    return 0;
}

// record a move
void set_move(game_record_list_t *lst, int game_idx, int step, uint8_t pos)
{
    game_record_t *rec = &lst->data[game_idx];
    size_t idx = step / 2;
    if (step % 2 == 0)
        rec->moves[idx] = (rec->moves[idx] & 0xF0) | (pos & 0x0F);
    else
        rec->moves[idx] = (rec->moves[idx] & 0x0F) | ((pos & 0x0F) << 4);
    rec->move_count = step + 1;
}

// read a move
uint8_t get_move(const game_record_list_t *lst, int game_idx, int step)
{
    const game_record_t *rec = &lst->data[game_idx];
    size_t idx = step / 2;
    if (step % 2 == 0)
        return rec->moves[idx] & 0x0F;
    else
        return (rec->moves[idx] >> 4) & 0x0F;
}

void print_game_list(const game_record_list_t *lst)
{
    for (size_t i = 0; i < lst->size; i++) {
        printf("Game %zu Moves:", i + 1);
        for (int s = 0; s < lst->data[i].move_count; s++) {
            uint8_t idx = get_move(lst, i, s);
            printf(" %s", pos_table[idx]);
            if (s + 1 < lst->data[i].move_count)
                printf(" ->");
        }
        printf("\n");
    }
}
