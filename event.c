#include <windows.h>
#include <conio.h>
#include <ctype.h>
#include "event.h"
#include "board.h"
#include "rules.h"
#include "ai.h"
#include "ui.h"

static void flushInputBuffer(void) {
    while (_kbhit()) {
        int extra = _getch();
        if (extra == 0 || extra == 224 || extra == -32) {
            if (_kbhit()) {
                (void)_getch();
            }
        }
    }
}

RawKey pollRawKey(void) {
    if (!_kbhit()) return RAW_NONE;
    int ch = _getch();

    RawKey result = RAW_NONE;
    if (ch == 0 || ch == 224 || ch == -32) {
        if (!_kbhit()) return RAW_NONE;
        ch = _getch();
        if (ch == 72) result = RAW_UP;
        else if (ch == 80) result = RAW_DOWN;
        else if (ch == 75) result = RAW_LEFT;
        else if (ch == 77) result = RAW_RIGHT;
    } else {
        switch (tolower(ch)) {
            case 'w': result = RAW_W; break;
            case 's': result = RAW_S; break;
            case 'a': result = RAW_A; break;
            case 'd': result = RAW_D; break;
            case 13:
            case ' ': result = RAW_ENTER; break;
            case 27:  result = RAW_ESC; break;
            case 'z': result = RAW_Z; break;
            case 'l': result = RAW_L; break;
            case 't': result = RAW_T; break;
            case 'h': result = RAW_H; break;
            case 'r': result = RAW_R; break;
            case 'm': result = RAW_M; break;
        }
    }

    flushInputBuffer();
    return result;
}

GameAction mapkeytoAction(RawKey key) {
    if (key == RAW_NONE)  return ACT_NONE;
    if (key == RAW_ENTER) return ACT_SELECT;
    if (key == RAW_ESC)   return ACT_BACK;
    if (key == RAW_Z)     return ACT_UNDO;
    if (key == RAW_L)     return ACT_SAVE;
    if (key == RAW_T)     return ACT_LOAD;
    if (key == RAW_H)     return ACT_HINT;
    if (key == RAW_R)     return ACT_RESTART;
    if (key == RAW_M)     return ACT_MUTE;

    if (g_game.state != STATE_PLAYING) {
        if (key == RAW_W || key == RAW_UP)    return ACT_UP;
        if (key == RAW_S || key == RAW_DOWN)  return ACT_DOWN;
        if (key == RAW_A || key == RAW_LEFT)  return ACT_LEFT;
        if (key == RAW_D || key == RAW_RIGHT) return ACT_RIGHT;
        return ACT_NONE;
    }

    ControlScheme activeScheme = (g_game.currentTurn == _X)
                                 ? g_game.settings.P1_control
                                 : g_game.settings.P2_control;

    if (activeScheme == KEYSET_WASD) {
        if (key == RAW_W) return ACT_UP;
        if (key == RAW_S) return ACT_DOWN;
        if (key == RAW_A) return ACT_LEFT;
        if (key == RAW_D) return ACT_RIGHT;
    } else {
        if (key == RAW_UP)    return ACT_UP;
        if (key == RAW_DOWN)  return ACT_DOWN;
        if (key == RAW_LEFT)  return ACT_LEFT;
        if (key == RAW_RIGHT) return ACT_RIGHT;
    }
    return ACT_NONE;
}

static void MainScreen(GameAction act) {
    int total = 6;
    if (act == ACT_UP) g_game.menuIndex = (g_game.menuIndex - 1 + total) % total;
    else if (act == ACT_DOWN) g_game.menuIndex = (g_game.menuIndex + 1) % total;
    else if (act == ACT_SELECT) {
        int selected = g_game.menuIndex;
        g_game.menuIndex = 0;
        if (selected == 0)      g_game.state = STATE_PLAY_MODE;
        else if (selected == 1) {
            if (PromptLoadGame(25)) {
                Sleep(700);
                g_game.state = STATE_PLAYING;
            } else {
                Sleep(1000);
                ClearLine(25); ClearLine(26);
            }
        }
        else if (selected == 2) g_game.state = STATE_ABOUT;
        else if (selected == 3) g_game.state = STATE_SETTINGS;
        else if (selected == 4) g_game.state = STATE_TUTORIAL;
        else if (selected == 5) g_game.state = STATE_QUIT;
    }
}

static bool makeMove(int r, int c) {
    if (!valid(r, c) || g_game.board[r][c] != EMPTY) {
        PlayBeep(200, 70);
        return false;
    }
    place(r, c, g_game.currentTurn);
    DrawCell(r, c, true);
    UpdatePanelStats();
    PlayBeep(g_game.currentTurn == _X ? 700 : 900, 40);

    GameResult res = check_result(r, c, &g_game.winner);
    if (res == RESULT_WIN) {
        g_game.isWinning = true;
        FlashWinCells();
        return true;
    }
    if (res == RESULT_DRAW) {
        g_game.winner = EMPTY;
        g_game.isWinning = true;
        FlashDrawBoard();
        return true;
    }
    g_game.currentTurn = (g_game.currentTurn == _X) ? _Y : _X;
    UpdatePanelStats();
    return true;
}

static void makeAIMove(void) {
    if (g_game.isWinning) return;

    int savedX = g_game.CursorX;
    int savedY = g_game.CursorY;
    int botR = -1, botC = -1;

    bot_move(g_game.currentTurn, &botR, &botC);
    flushInputBuffer();

    if (valid(botR, botC) && g_game.board[botR][botC] == EMPTY) {
        place(botR, botC, g_game.currentTurn);
        DrawCell(botR, botC, false);
        UpdatePanelStats();
        PlayBeep(900, 40);

        GameResult res = check_result(botR, botC, &g_game.winner);
        if (res == RESULT_WIN) {
            g_game.isWinning = true;
            FlashWinCells();
        } else if (res == RESULT_DRAW) {
            g_game.winner = EMPTY;
            g_game.isWinning = true;
            FlashDrawBoard();
        } else {
            g_game.currentTurn = _X;
            UpdatePanelStats();
        }
    }

    g_game.CursorX = savedX;
    g_game.CursorY = savedY;
    DrawCell(g_game.CursorY, g_game.CursorX, true);
}

static void resetTimeCounter(void) {
    g_game.remainTime = (float)g_game.settings.TurnTimeOut;
}

void Playing(GameAction act, float dt) {
    int msgRow = 2 * BOARD_SIZE + TOP + 2;
    bool isHumanTurn = (!g_game.isVsAI) || (g_game.currentTurn == _X);

    if (g_game.settings.TurnTimeOut > 0 && isHumanTurn) {
        g_game.remainTime -= dt;
        if (g_game.remainTime <= 0.0f) {
            g_game.remainTime = 0.0f;
            g_game.isWinning = true;
            g_game.winner = (g_game.currentTurn == _X) ? _Y : _X;
            g_game.state = STATE_GAMEOVER;
            return;
        }
    }

    if (act == ACT_NONE) return;

    if (act == ACT_UP || act == ACT_DOWN || act == ACT_LEFT || act == ACT_RIGHT) {
        int oldX = g_game.CursorX;
        int oldY = g_game.CursorY;

        if (act == ACT_UP && g_game.CursorY > 0) g_game.CursorY--;
        else if (act == ACT_DOWN && g_game.CursorY < BOARD_SIZE - 1) g_game.CursorY++;
        else if (act == ACT_LEFT && g_game.CursorX > 0) g_game.CursorX--;
        else if (act == ACT_RIGHT && g_game.CursorX < BOARD_SIZE - 1) g_game.CursorX++;

        if (oldX != g_game.CursorX || oldY != g_game.CursorY) {
            DrawCell(oldY, oldX, false);
            DrawCell(g_game.CursorY, g_game.CursorX, true);
        }
    }
    else if (act == ACT_SELECT) {
        int targetR = g_game.CursorY;
        int targetC = g_game.CursorX;

        if (makeMove(targetR, targetC)) {
            flushInputBuffer();
            if (g_game.isWinning) {
                g_game.state = STATE_GAMEOVER;
                return;
            }
            resetTimeCounter();
            if (g_game.isVsAI) {
                makeAIMove();
                flushInputBuffer();
                if (g_game.isWinning) {
                    g_game.state = STATE_GAMEOVER;
                    return;
                }
                resetTimeCounter();
            }
        }
    }
    else if (act == ACT_BACK) {
        g_game.menuIndex = 0;
        g_game.state = STATE_PAUSE;
    }
    else if (act == ACT_SAVE) {
        PromptSaveGame(msgRow);
        Sleep(1000);
        ClearLine(msgRow); ClearLine(msgRow + 1);
        flushInputBuffer();
    }
    else if (act == ACT_LOAD) {
        if (PromptLoadGame(msgRow)) {
            Sleep(800);
            DrawGameScreen();
        } else {
            Sleep(1000);
            ClearLine(msgRow); ClearLine(msgRow + 1);
        }
        flushInputBuffer();
    }
    else if (act == ACT_HINT) {
        int hintR = -1, hintC = -1;
        bot_move(g_game.currentTurn, &hintR, &hintC);
        if (valid(hintR, hintC)) {
            int oldX = g_game.CursorX, oldY = g_game.CursorY;
            g_game.CursorY = hintR;
            g_game.CursorX = hintC;
            DrawCell(oldY, oldX, false);
            DrawCell(g_game.CursorY, g_game.CursorX, true);
            ShowMsg(msgRow, 14, "Goi y: Con tro da chi vao o tot nhat!");
        }
    }
    else if (act == ACT_RESTART) {
        startNewMatch(g_game.isVsAI);
        DrawGameScreen();
    }
    else if (act == ACT_UNDO) {
        if (undo_move()) {
            if (g_game.isVsAI && move_count > 0) {
                undo_move();
            } else if (!g_game.isVsAI) {
                g_game.currentTurn = (g_game.currentTurn == _X) ? _Y : _X;
            }
            resetTimeCounter();
            DrawGameScreen();
        }
    }
}

static void PlayMode(GameAction act) {
    int total = 2;
    if (act == ACT_UP) g_game.menuIndex = (g_game.menuIndex - 1 + total) % total;
    else if (act == ACT_DOWN) g_game.menuIndex = (g_game.menuIndex + 1) % total;
    else if (act == ACT_BACK) {
        g_game.state = STATE_MENU;
        g_game.menuIndex = 0;
    }
    else if (act == ACT_SELECT) {
        bool isVsAI = (g_game.menuIndex == 1);
        startNewMatch(isVsAI);
        g_game.remainTime = (float)g_game.settings.TurnTimeOut;
        g_game.state = STATE_PLAYING;
    }
}

static void SettingsMenu(GameAction act) {
    int totalSettings = 6;
    if (act == ACT_UP) g_game.menuIndex = (g_game.menuIndex - 1 + totalSettings) % totalSettings;
    else if (act == ACT_DOWN) g_game.menuIndex = (g_game.menuIndex + 1) % totalSettings;
    else if (act == ACT_BACK) {
        g_game.menuIndex = 3;
        g_game.state = STATE_MENU;
    }
    else if (act == ACT_LEFT || act == ACT_RIGHT || act == ACT_SELECT) {
        int dir = (act == ACT_LEFT) ? -1 : 1;
        switch (g_game.menuIndex) {
            case 0:
                g_game.settings.SoundEnabled = !g_game.settings.SoundEnabled;
                break;
            case 1:
                g_game.settings.P1_control = (g_game.settings.P1_control == KEYSET_WASD) ? KEYSET_ARROWS : KEYSET_WASD;
                break;
            case 2:
                g_game.settings.P2_control = (g_game.settings.P2_control == KEYSET_ARROWS) ? KEYSET_WASD : KEYSET_ARROWS;
                break;
            case 3: {
                int TimeOptions[] = { 15, 30, 60, 0 };
                int currentIdx = 0;
                for (int i = 0; i < 4; i++) {
                    if (g_game.settings.TurnTimeOut == TimeOptions[i]) { currentIdx = i; break; }
                }
                currentIdx = (currentIdx + dir + 4) % 4;
                g_game.settings.TurnTimeOut = TimeOptions[currentIdx];
                break;
            }
            case 4:
                g_game.settings.color_P1 = (g_game.settings.color_P1 + dir + 4) % 4;
                break;
            case 5:
                g_game.settings.color_P2 = (g_game.settings.color_P2 + dir + 4) % 4;
                break;
        }
    }
}

static void InfoScreen(GameAction act, int returnMenuIndex) {
    if (act == ACT_BACK || act == ACT_SELECT) {
        g_game.menuIndex = returnMenuIndex;
        g_game.state = STATE_MENU;
    }
}

static void PauseGame(GameAction act) {
    int total = 3;
    if (act == ACT_UP) g_game.menuIndex = (g_game.menuIndex - 1 + total) % total;
    else if (act == ACT_DOWN) g_game.menuIndex = (g_game.menuIndex + 1) % total;
    else if (act == ACT_BACK) g_game.state = STATE_PLAYING;
    else if (act == ACT_SELECT) {
        if (g_game.menuIndex == 0) g_game.state = STATE_PLAYING;
        else if (g_game.menuIndex == 1) {
            startNewMatch(g_game.isVsAI);
            g_game.remainTime = (float)g_game.settings.TurnTimeOut;
            g_game.state = STATE_PLAYING;
        }
        else if (g_game.menuIndex == 2) {
            g_game.menuIndex = 0;
            g_game.state = STATE_MENU;
        }
    }
}

static void GameOver(GameAction act) {
    int total = 2;
    if (act == ACT_UP) g_game.menuIndex = (g_game.menuIndex - 1 + total) % total;
    else if (act == ACT_DOWN) g_game.menuIndex = (g_game.menuIndex + 1) % total;
    else if (act == ACT_SELECT) {
        if (g_game.menuIndex == 0) {
            startNewMatch(g_game.isVsAI);
            g_game.remainTime = (float)g_game.settings.TurnTimeOut;
            g_game.state = STATE_PLAYING;
        }
        else if (g_game.menuIndex == 1) {
            g_game.menuIndex = 0;
            g_game.state = STATE_MENU;
        }
    }
}

void startNewMatch(bool vsAI) {
    board_reset();
    g_game.currentTurn = _X;
    g_game.CursorX = BOARD_SIZE / 2;
    g_game.CursorY = BOARD_SIZE / 2;
    g_game.winner = EMPTY;
    g_game.isWinning = false;
    g_game.isVsAI = vsAI;
    g_game.aiSide = _Y;
    g_game.remainTime = (float)g_game.settings.TurnTimeOut;
    flushInputBuffer();
}

void dispatchGameFlow(float dt) {
    RawKey key = pollRawKey();
    GameAction action = mapkeytoAction(key);

    if (action == ACT_MUTE) {
        g_game.settings.SoundEnabled = !g_game.settings.SoundEnabled;
        if (g_game.state == STATE_PLAYING) UpdatePanelStats();
    }

    switch (g_game.state) {
        case STATE_MENU:      MainScreen(action); break;
        case STATE_PLAY_MODE: PlayMode(action); break;
        case STATE_SETTINGS:  SettingsMenu(action); break;
        case STATE_ABOUT:     InfoScreen(action, 2); break;
        case STATE_TUTORIAL:  InfoScreen(action, 4); break;
        case STATE_PLAYING:   Playing(action, dt); break;
        case STATE_PAUSE:     PauseGame(action); break;
        case STATE_GAMEOVER:  GameOver(action); break;
        default: break;
    }
}