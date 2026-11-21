#include <string.h>
#include "board.h"

int  hist_r[BOARD_SIZE * BOARD_SIZE];
int  hist_c[BOARD_SIZE * BOARD_SIZE];
int  move_count;
int  win_r[BOARD_SIZE];
int  win_c[BOARD_SIZE];
int  win_len;

bool valid(int r, int c)
{
    return r >= 0 && c >= 0 && r < BOARD_SIZE && c < BOARD_SIZE;
}

void board_reset(void){
    for (int i = 0; i < BOARD_SIZE; i++){
        for (int j = 0; j < BOARD_SIZE; j++){
            g_game.board[i][j] = EMPTY;
        }
    }
    move_count = 0;
    win_len = 0;
}

void place(int r, int c, CellState player)
{
    g_game.board[r][c] = player;
    hist_r[move_count] = r;
    hist_c[move_count] = c;
    move_count++;
}

bool undo_move(void)
{
    if (move_count == 0) return false;
    move_count--;
    g_game.board[hist_r[move_count]][hist_c[move_count]] = EMPTY;
    win_len = 0;
    return true;
}