#include <stdlib.h>
#include "ai.h"
#include "board.h"

#define BORDER         2              
#define FIVE_SCORE     1000000000LL   
#define WIN_THRESHOLD   100000000LL   
#define INF          100000000000LL   


/* PATTERN_SCORE[chuỗi dài bao nhiêu][check đuôi với đít][đang lượt nào ] */
static const long long PATTERN_SCORE[5][3][2] = {
    [1] = { [1] = { 1, 1 },[2] = { 5, 10 } },
    [2] = { [1] = { 10, 50 },[2] = { 100, 1000 } },
    [3] = { [1] = { 500, 5000 },[2] = { 10000, 500000 } },
    [4] = { [1] = { 100000, 5000000 },[2] = { 1000000, 5000000 } },
};
static inline int to_sign(CellState p) { //ham chuyen doi 
    if (p == _X) return 1;
    if (p == _Y) return -1;
    return 0;
}
static long long consecutive_score(int len, int open_ends, bool is_my_turn)
{
    if (len >= WIN_LEN)
        return (block_rule && open_ends == 0) ? 0 : FIVE_SCORE;
    if (open_ends == 0)
        return 0;
    return PATTERN_SCORE[len][open_ends][is_my_turn ? 1 : 0];
}

/* getscore là để m tính xem chuỗi đó chặn 0,1,2 đầu còn scan là tạo chuỗi 1d để m check mấy đuôi phụ phần getscore*/
static long long get_score(const int a[], int player)
{
    const int size = BOARD_SIZE + 2;
    long long score = 0;

    for (int i = 1; i < size - 1; i++) {
        if (abs(a[i]) != 1) continue;

        int j = i;
        while (j + 1 < size - 1 && a[j + 1] == a[i]) j++;

        int len = j - i + 1;
        int open_ends = 2 - (a[i - 1] != 0) - (a[j + 1] != 0);
        score += consecutive_score(len, open_ends, player == a[i]) * a[i];
        i = j;
    }
    return score;
}

static long long scan_line(int r0, int c0, int dr, int dc, int player)
{
    int b[BOARD_SIZE + 2];
    for (int k = 0; k < BOARD_SIZE + 2; k++) {
        int r = r0 + k * dr, c = c0 + k * dc;
        b[k] = valid(r, c) ? to_sign(g_game.board[r][c]) : BORDER;
    }
    return get_score(b, player);
}



#define SCAN(r0, c0, dr, dc)                                          \
    do {                                                              \
        score += scan_line((r0), (c0), (dr), (dc), player);           \
        if (llabs(score) >= WIN_THRESHOLD) return score;              \
    } while (0)

/* này là để đánh giá xong đưa ra điểm cụ thể ví dụ như là tổng của thằng X á mà > 0 thì nó có lợi, < 0 thì bất lợi, tương tự với thằng O */
static long long evaluation(CellState player)
{   
    int player_sign = to_sign(player);
    long long score = 0;
    for (int i = 0; i <= BOARD_SIZE; i++)     SCAN(i, -1, 0, 1);                 /* hàng ngang */
    for (int j = 0; j <= BOARD_SIZE; j++)     SCAN(-1, j, 1, 0);                 /* cột dọc    */
    for (int i = -1; i <= BOARD_SIZE; i++)     SCAN(i, -1, 1, 1);                 /* chéo chính */
    for (int j = 0; j <= BOARD_SIZE; j++)     SCAN(-1, j, 1, 1);
    for (int i = 0; i <= BOARD_SIZE + 1; i++) SCAN(i, -1, -1, 1);                /* chéo phụ   */
    for (int j = 0; j <= BOARD_SIZE; j++)     SCAN(BOARD_SIZE, j, -1, 1);
    return score;
}



static int cand_r[BOARD_SIZE * BOARD_SIZE];
static int cand_c[BOARD_SIZE * BOARD_SIZE];
static int cand_count;
static int max_depth;

/* Lọc ô nằm ngay gần đó . lọc 8 hướng thôi ( lọc nhiều quá chịu k nổi =)) ) */
static void gen_candidates(void)
{
    cand_count = 0;
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (g_game.board[r][c] != EMPTY) continue;

            bool near_stone = false;
            for (int dr = -1; dr <= 1 && !near_stone; dr++)
                for (int dc = -1; dc <= 1; dc++)
                    if (valid(r + dr, c + dc) && g_game.board[r + dr][c + dc] != EMPTY) {
                        near_stone = true;
                        break;
                    }

            if (near_stone) {
                cand_r[cand_count] = r;
                cand_c[cand_count] = c;
                cand_count++;
            }
        }
    }
}

/* này là để quyết định nước đi sao cho có lợi nhất. */
static long long minimax(int depth, long long alpha, long long beta,
    CellState player, int* x, int* y)
{
    long long val = evaluation(player);
    if (depth == 0 || llabs(val) >= WIN_THRESHOLD)
        return val;

    bool maximizing = (player == _X);
    long long best = maximizing ? -INF : INF;
    CellState next_player = (player == _X) ? _Y : _X;

    for (int k = 0; k < cand_count; k++) {
        int i = cand_r[k], j = cand_c[k];
        if (g_game.board[i][j] != EMPTY) continue;

        g_game.board[i][j] = player;
        long long eval = minimax(depth - 1, alpha, beta, next_player, x, y);
        g_game.board[i][j] = EMPTY;

        if (maximizing ? eval > best : eval < best) {
            best = eval;
            if (depth == max_depth) { *x = i; *y = j; }
        }
        if (maximizing) { if (eval > alpha) alpha = eval; }
        else { if (eval < beta)  beta = eval; }
        if (beta <= alpha) break;                      
    }
    return best;
}



static const int DX[8] = { -1, 1, 0, 0, -1, 1, 1, -1 };
static const int DY[8] = { 0, 0, -1, 1, 1, -1, 1, -1 };


static bool random_adjacent(int* x, int* y)
{
    int start = rand() % 8;
    for (int t = 0; t < 8; t++) {
        int d = (start + t) % 8;
        int nx = *x + DX[d], ny = *y + DY[d];
        if (valid(nx, ny) && g_game.board[nx][ny] == EMPTY) {
            *x = nx;
            *y = ny;
            return true;
        }
    }
    return false;
}
/* Nước đầu tiên là đánh vô giữa bàn cờ, Nước thứ hai là đánh random vào 8 ô xung quanh bất kì , Nước thứ 3 là dùng hàm gen_candidates để lọc danh sách thỏa điều kiện */
  
    
void bot_move(CellState bot, int* x, int* y)
{
    if (move_count == 0) {                       
        *x = *y = BOARD_SIZE  / 2;
        return;
    }

    *x = hist_r[move_count - 1];                 
    *y = hist_c[move_count - 1];

    if (move_count == 1) {                      
        random_adjacent(x, y);
    }
    else {
        gen_candidates();
        max_depth = (cand_count <= 110) ? 3 : 2; 
        minimax(max_depth, -INF, INF, bot, x, y);
    }

    if (!valid(*x, *y) || g_game.board[*x][*y] != EMPTY) {     
        if (!random_adjacent(x, y)) {
            for (int r = 0; r < BOARD_SIZE; r++)
                for (int c = 0; c < BOARD_SIZE; c++)
                    if (g_game.board[r][c] == EMPTY) { *x = r; *y = c; return; }
        }
    }
}