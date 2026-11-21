#include <stdlib.h>
#include <stdbool.h>
#include "ai.h"
#include "board.h"

#define BORDER        2
#define FIVE_SCORE    1000000000LL
#define WIN_THRESHOLD  100000000LL
#define INF           100000000000LL

static const long long PATTERN_SCORE[5][3][2] = {
    [1] = { [1] = { 2, 4 },         [2] = { 10, 20 } },
    [2] = { [1] = { 20, 80 },       [2] = { 200, 1500 } },
    [3] = { [1] = { 800, 6000 },    [2] = { 15000, 600000 } },
    [4] = { [1] = { 150000, 8000000 }, [2] = { 2000000, 10000000 } },
};

static inline int to_sign(CellState p) {
    if (p == _X) return 1;
    if (p == _Y) return -1;
    return 0;
}

static long long consecutive_score(int len, int open_ends, bool is_my_turn) {
    if (len >= WIN_LEN)
        return FIVE_SCORE;
    if (open_ends == 0)
        return 0;
    return PATTERN_SCORE[len][open_ends][is_my_turn ? 1 : 0];
}

static long long get_score(const int a[], int length, int turn_sign) {
    long long score = 0;
    for (int i = 1; i < length - 1; i++) {
        if (abs(a[i]) != 1) continue;
        int j = i;
        while (j + 1 < length - 1 && a[j + 1] == a[i]) j++;

        int len = j - i + 1;
        int open_ends = 2 - (a[i - 1] != 0) - (a[j + 1] != 0);
        score += consecutive_score(len, open_ends, turn_sign == a[i]) * a[i];
        i = j;
    }
    return score;
}

static long long scan_line(int r0, int c0, int dr, int dc, int steps, int turn_sign) {
    int b[BOARD_SIZE + 2];
    b[0] = BORDER;
    for (int k = 0; k < steps; k++) {
        b[k + 1] = to_sign(g_game.board[r0 + k * dr][c0 + k * dc]);
    }
    b[steps + 1] = BORDER;
    return get_score(b, steps + 2, turn_sign);
}

#define SCAN(r0, c0, dr, dc, steps)                                          \
    do {                                                                     \
        score += scan_line((r0), (c0), (dr), (dc), (steps), turn_sign);      \
        if (llabs(score) >= WIN_THRESHOLD) return score;                     \
    } while (0)

static long long evaluation(CellState turn) {
    int turn_sign = to_sign(turn);
    long long score = 0;

    for (int r = 0; r < BOARD_SIZE; r++) SCAN(r, 0, 0, 1, BOARD_SIZE);
    for (int c = 0; c < BOARD_SIZE; c++) SCAN(0, c, 1, 0, BOARD_SIZE);

    for (int r = 0; r <= BOARD_SIZE - WIN_LEN; r++) SCAN(r, 0, 1, 1, BOARD_SIZE - r);
    for (int c = 1; c <= BOARD_SIZE - WIN_LEN; c++) SCAN(0, c, 1, 1, BOARD_SIZE - c);

    for (int r = WIN_LEN - 1; r < BOARD_SIZE; r++) SCAN(r, 0, -1, 1, r + 1);
    for (int c = 1; c <= BOARD_SIZE - WIN_LEN; c++) SCAN(BOARD_SIZE - 1, c, -1, 1, BOARD_SIZE - c);

    return score;
}

typedef struct {
    int r, c;
    long long priority;
} MoveCand;

static long long quick_heuristic(int r, int c) {
    static const int dirs[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    long long h = 0;

    for (int d = 0; d < 4; d++) {
        int dr = dirs[d][0], dc = dirs[d][1];
        for (int side = 1; side <= 2; side++) {
            CellState target = (side == 1) ? _X : _Y;
            int cnt = 0, open = 0;

            int i = 1;
            while (i < WIN_LEN && valid(r + dr * i, c + dc * i) && g_game.board[r + dr * i][c + dc * i] == target) {
                cnt++; i++;
            }
            if (valid(r + dr * i, c + dc * i) && g_game.board[r + dr * i][c + dc * i] == EMPTY) open++;

            int j = 1;
            while (j < WIN_LEN && valid(r - dr * j, c - dc * j) && g_game.board[r - dr * j][c - dc * j] == target) {
                cnt++; j++;
            }
            if (valid(r - dr * j, c - dc * j) && g_game.board[r - dr * j][c - dc * j] == EMPTY) open++;

            if (cnt >= WIN_LEN - 1) h += 1000000;
            else if (cnt > 0) h += (long long)cnt * cnt * (open + 1) * 50;
        }
    }
    int center = BOARD_SIZE / 2;
    h += (BOARD_SIZE - (abs(r - center) + abs(c - center)));
    return h;
}

static int cmp_cand(const void *a, const void *b) {
    long long pa = ((const MoveCand *)a)->priority;
    long long pb = ((const MoveCand *)b)->priority;
    return (pb > pa) - (pb < pa);
}

static int gen_candidates(MoveCand out[]) {
    int count = 0;
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (g_game.board[r][c] != EMPTY) continue;

            bool near_stone = false;
            for (int dr = -2; dr <= 2 && !near_stone; dr++) {
                for (int dc = -2; dc <= 2; dc++) {
                    if ((dr == 0 && dc == 0) || (abs(dr) == 2 && abs(dc) == 2)) continue;
                    if (valid(r + dr, c + dc) && g_game.board[r + dr][c + dc] != EMPTY) {
                        near_stone = true;
                        break;
                    }
                }
            }
            if (near_stone) {
                out[count].r = r;
                out[count].c = c;
                out[count].priority = quick_heuristic(r, c);
                count++;
            }
        }
    }
    if (count > 1) qsort(out, count, sizeof(MoveCand), cmp_cand);
    return count;
}

static int max_depth, best_move_r, best_move_c;

static long long minimax(int depth, long long alpha, long long beta, CellState player) {
    long long val = evaluation(player);
    if (depth == 0 || llabs(val) >= WIN_THRESHOLD) return val;

    MoveCand cands[BOARD_SIZE * BOARD_SIZE];
    int cand_count = gen_candidates(cands);
    if (cand_count == 0) return val;

    int limit = (cand_count > 18) ? 18 : cand_count;
    bool maximizing = (player == _X);
    long long best = maximizing ? -INF : INF;
    CellState next_player = (player == _X) ? _Y : _X;

    for (int k = 0; k < limit; k++) {
        int r = cands[k].r, c = cands[k].c;
        g_game.board[r][c] = player;
        long long eval = minimax(depth - 1, alpha, beta, next_player);
        g_game.board[r][c] = EMPTY;

        if (maximizing ? (eval > best) : (eval < best)) {
            best = eval;
            if (depth == max_depth) { best_move_r = r; best_move_c = c; }
        }
        if (maximizing) { if (eval > alpha) alpha = eval; }
        else            { if (eval < beta)  beta = eval; }
        if (beta <= alpha) break;
    }
    return best;
}

static const int DR[8] = { -1, 1, 0, 0, -1, 1, 1, -1 };
static const int DC[8] = { 0, 0, -1, 1, 1, -1, 1, -1 };

static bool random_adjacent(int r, int c, int *out_r, int *out_c) {
    int start = rand() % 8;
    for (int t = 0; t < 8; t++) {
        int d = (start + t) % 8;
        int nr = r + DR[d], nc = c + DC[d];
        if (valid(nr, nc) && g_game.board[nr][nc] == EMPTY) {
            *out_r = nr; *out_c = nc;
            return true;
        }
    }
    return false;
}

void bot_move(CellState bot, int* x, int* y) {
    if (move_count == 0) {
        *x = *y = BOARD_SIZE / 2;
        return;
    }

    int last_r = hist_r[move_count - 1];
    int last_c = hist_c[move_count - 1];
    best_move_r = -1;
    best_move_c = -1;

    if (move_count == 1) {
        random_adjacent(last_r, last_c, &best_move_r, &best_move_c);
    } else {
        MoveCand cands[BOARD_SIZE * BOARD_SIZE];
        int cnt = gen_candidates(cands);
        if (cnt > 0) {
            best_move_r = cands[0].r;
            best_move_c = cands[0].c;
            max_depth = (cnt <= 30) ? 3 : 2;
            minimax(max_depth, -INF, INF, bot);
        }
    }

    if (!valid(best_move_r, best_move_c) || g_game.board[best_move_r][best_move_c] != EMPTY) {
        if (!random_adjacent(last_r, last_c, &best_move_r, &best_move_c)) {
            for (int r = 0; r < BOARD_SIZE; r++) {
                for (int c = 0; c < BOARD_SIZE; c++) {
                    if (g_game.board[r][c] == EMPTY) { *x = r; *y = c; return; }
                }
            }
        }
    }

    *x = best_move_r;
    *y = best_move_c;
}