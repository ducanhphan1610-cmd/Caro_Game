#ifndef CARO_H
#define CARO_H
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>

#define BOARD_SIZE 15 
#define CELL_SIZE 40
#define OFFSET_X 50
#define OFFSET_Y 50

typedef enum {
    EMPTY = 0,
    _X = 1,
    _Y = 2
} CellState;

typedef struct{
    CellState board[BOARD_SIZE][BOARD_SIZE];
    CellState currentTurn;
    int winner;
    bool isWinning;
    char gamename[10];
} Game;

extern Game g_game;
#endif
