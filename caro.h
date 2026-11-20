#ifndef CARO_H
#define CARO_H

#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>

#define BOARD_SIZE 15 
#define WIN_LEN 5
#define CELL_SIZE 40
#define OFFSET_X 50
#define OFFSET_Y 50
typedef enum{
    STATE_MENU = 0,
    STATE_PLAY_MODE,
    STATE_ABOUT,
    STATE_TUTORIAL,
    STATE_SETTINGS,
    STATE_PLAYING,
    STATE_PAUSE,
    STATE_GAMEOVER,
    STATE_QUIT,
} GameState;
typedef enum{
    KEYSET_WASD = 0,
    KEYSET_ARROWS = 1,
} ControlScheme;
typedef struct{
    bool SoundEnabled;
    ControlScheme P1_control;
    ControlScheme P2_control;
    int TurnTimeOut;
    int color_P1;
    int color_P2;
} Settings;
typedef enum {
    EMPTY = 0,
    _X = 1,
    _Y = 2
} CellState;

typedef struct{
    CellState board[BOARD_SIZE][BOARD_SIZE];
    CellState currentTurn;
    CellState aiSide;
    int winner;
    int CursorX;
    int CursorY;
    bool isWinning;
    bool isVsAI;
    int menuIndex;
    GameState state;
    Settings settings;
    float remainTime;
    char gamename[10];
} Game;

extern Game g_game;
#endif
