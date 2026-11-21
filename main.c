#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "caro.h"
#include "event.h"
#include "board.h"
#include "rules.h"
#include "ui.h"

Game g_game;

int SaveGame(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) return 0;
    fprintf(f, "%s\n", g_game.gamename);
    fprintf(f, "%d %d %d %.2f\n", g_game.currentTurn, g_game.isVsAI ? 1 : 0, move_count, g_game.remainTime);
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            fprintf(f, "%d ", g_game.board[i][j]);
        }
        fprintf(f, "\n");
    }
    for (int k = 0; k < move_count; k++) {
        fprintf(f, "%d %d\n", hist_r[k], hist_c[k]);
    }
    fclose(f);
    return 1;
}

int LoadGame(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) return 0;

    char loadedName[20];
    if (fscanf(f, " %19[^\n]", loadedName) != 1) {
        fclose(f);
        return 0;
    }
    int turn = 1, vsAI = 0, loadedMoves = 0;
    float remTime = (float)g_game.settings.TurnTimeOut;
    if (fscanf(f, "%d %d %d %f", &turn, &vsAI, &loadedMoves, &remTime) != 4) {
        fclose(f);
        return 0;
    }

    CellState tempBoard[BOARD_SIZE][BOARD_SIZE];
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            int cell = 0;
            if (fscanf(f, " %d", &cell) != 1 || cell < 0 || cell > 2) {
                fclose(f);
                return 0;
            }
            tempBoard[i][j] = (CellState)cell;
        }
    }

    board_reset();
    strcpy(g_game.gamename, loadedName);
    g_game.currentTurn = (CellState)turn;
    g_game.isVsAI = (vsAI == 1);
    g_game.aiSide = _Y;
    g_game.remainTime = remTime;
    g_game.winner = EMPTY;
    g_game.isWinning = false;
    g_game.CursorX = BOARD_SIZE / 2;
    g_game.CursorY = BOARD_SIZE / 2;

    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            g_game.board[i][j] = tempBoard[i][j];

    move_count = loadedMoves;
    for (int k = 0; k < move_count; k++) {
        fscanf(f, "%d %d", &hist_r[k], &hist_c[k]);
    }
    fclose(f);
    return 1;
}

static bool InputFileNameUI(const char* prompt, char* buf, int size, int row) {
    ClearLine(row); ClearLine(row + 1);
    GotoXY(3, row);
    SetColor(14); printf("%s", prompt);
    SetColor(15);
    SetCursorVisible(true);
    bool ok = (fgets(buf, size, stdin) != NULL);
    SetCursorVisible(false);
    SetColor(7);
    if (!ok) return false;
    buf[strcspn(buf, "\r\n")] = 0;
    return buf[0] != '\0';
}

bool PromptSaveGame(int msgRow) {
    char filename[100];
    if (!InputFileNameUI("Nhap ten tap tin de luu (Enter de huy): ", filename, sizeof(filename), msgRow)) {
        ClearLine(msgRow);
        return false;
    }
    if (SaveGame(filename)) {
        ShowMsg(msgRow + 1, 10, "Da luu game thanh cong!");
        return true;
    }
    ShowMsg(msgRow + 1, 12, "Khong tao duoc tap tin!");
    return false;
}

bool PromptLoadGame(int msgRow) {
    char filename[100];
    if (!InputFileNameUI("Nhap ten tap tin can tai (Enter de huy): ", filename, sizeof(filename), msgRow)) {
        ClearLine(msgRow);
        return false;
    }
    if (LoadGame(filename)) {
        ShowMsg(msgRow + 1, 10, "Da tai game thanh cong!");
        return true;
    }
    ShowMsg(msgRow + 1, 12, "Tap tin khong ton tai hoac bi loi!");
    return false;
}

int main(void) {
    srand((unsigned int)time(NULL));
    InitConsoleUI();

    strcpy(g_game.gamename, "NHOM 10");
    g_game.settings.SoundEnabled = true;
    g_game.settings.TurnTimeOut = 30;
    g_game.settings.P1_control = KEYSET_WASD;
    g_game.settings.P2_control = KEYSET_ARROWS;
    g_game.settings.color_P1 = 0; // 0: Do, 1: Xanh Duong, 2: Xanh La, 3: Vang
    g_game.settings.color_P2 = 1;
    g_game.state = STATE_MENU;

    clock_t lastTime = clock();
    float renderTimer = 0.0f;

    RenderStateScreen();

    while (g_game.state != STATE_QUIT) {
        clock_t now = clock();
        float dt = (float)(now - lastTime) / CLOCKS_PER_SEC;
        lastTime = now;

        GameState prevState = g_game.state;
        int prevMenu = g_game.menuIndex;
        int prevX = g_game.CursorX, prevY = g_game.CursorY;
        Settings prevSettings = g_game.settings;

        dispatchGameFlow(dt);

        if (prevState != g_game.state) {
            if (g_game.state == STATE_PLAYING) DrawGameScreen();
            else RenderStateScreen();
        }
        else if (g_game.state == STATE_PLAYING) {
            if (prevX != g_game.CursorX || prevY != g_game.CursorY) {
                DrawCell(prevY, prevX, false);
                DrawCell(g_game.CursorY, g_game.CursorX, true);
            }
            renderTimer += dt;
            if (renderTimer >= 0.25f) {
                UpdateTimerDisplay();
                renderTimer = 0.0f;
            }
        }
        else {
            bool settingChanged = memcmp(&prevSettings, &g_game.settings, sizeof(Settings)) != 0;
            if (prevMenu != g_game.menuIndex || settingChanged) {
                RenderStateScreen();
            }
        }

        Sleep(15);
    }

    system("cls");
    SetCursorVisible(true);
    return 0;
}