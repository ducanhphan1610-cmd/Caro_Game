#include "rules.h"
#include "board.h"


static const int D4R[4] = {0, 1, 1,  1};
static const int D4C[4] = {1, 0, 1, -1};


static bool is_blocked(int r, int c)
{
    return !valid(r, c) || g_game.board[r][c] != EMPTY;
}

bool check_win(int r, int c)
{
    win_len = 0;
    if (!valid(r, c) || g_game.board[r][c] == EMPTY)
        return false;

    CellState p = g_game.board[r][c];

    for (int d = 0; d < 4; d++) {
        int dr = D4R[d], dc = D4C[d];

        
        int sr = r, sc = c;
        while (valid(sr - dr, sc - dc) && g_game.board[sr - dr][sc - dc] == p) {
            sr -= dr;
            sc -= dc;
        }

       
        int er = sr, ec = sc, len = 0;
        while (valid(er, ec) && g_game.board[er][ec] == p) {
            er += dr;
            ec += dc;
            len++;
        }

        
        if (len >= WIN_LEN) {
            bool b1 = is_blocked(sr - dr, sc - dc);   
            bool b2 = is_blocked(er, ec);             

            if (!(block_rule && b1 && b2)) {          
                win_len = len;
                for (int k = 0; k < len; k++) {
                    win_r[k] = sr + k * dr;
                    win_c[k] = sc + k * dc;
                }
                return true;
            }
        }
    }
    return false;
}

bool check_draw(void)
{
    return move_count >= BOARD_SIZE * BOARD_SIZE;
}

GameResult check_result(int r, int c, int *winner)
{
    if (winner) *winner = EMPTY;

    if (check_win(r, c)) {
        if (winner) *winner = g_game.board[r][c];
        return RESULT_WIN;
    }
    if (check_draw())
        return RESULT_DRAW;

    return RESULT_CONTINUE;
}