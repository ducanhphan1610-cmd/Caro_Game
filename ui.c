#include <windows.h>
#include <stdio.h>
#include "ui.h"
#include "board.h"

static const int WIN_COLORS[4] = { 12, 11, 10, 14 };

void SetColor(int pColor) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)pColor);
}

void SetCursorVisible(bool pVisible) {
    CONSOLE_CURSOR_INFO info = { 20, pVisible };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void FixConsoleWindow(void) {
    HWND consoleWindow = GetConsoleWindow();
    LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
    style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
    SetWindowLong(consoleWindow, GWL_STYLE, style);
}

void GotoXY(int x, int y) {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void PlayBeep(int pFreq, int pMs) {
    if (g_game.settings.SoundEnabled) Beep(pFreq, pMs);
}

void ClearLine(int y) {
    GotoXY(0, y);
    printf("%-104s", "");
}

void ShowMsg(int pRow, int pColor, const char* pText) {
    ClearLine(pRow);
    GotoXY(3, pRow);
    SetColor(pColor);
    printf("%s", pText);
    SetColor(7);
}

void InitConsoleUI(void) {
    system("mode con cols=106 lines=36");
    system("title Game Caro");
    FixConsoleWindow();
    SetCursorVisible(false);
}

static void DrawTitle(void) {
    const char* art[5] = {
        "   ____    _    ____   ___  ",
        "  / ___|  / \\  |  _ \\ / _ \\ ",
        " | |     / _ \\ | |_) | | | |",
        " | |___ / ___ \\|  _ <| |_| |",
        "  \\____/_/   \\_\\_| \\_\\\\___/ "
    };
    SetColor(14);
    for (int i = 0; i < 5; i++) {
        GotoXY(38, 2 + i);
        fputs(art[i], stdout);
    }
    SetColor(11);
    GotoXY(41, 8);
    printf("NHOM 10 - DO AN GAME CARO");
    SetColor(7);
}

static void DrawBox(int x, int y, int w, int h) {
    SetColor(8);
    GotoXY(x, y); printf("+");
    for (int i = 0; i < w - 2; i++) printf("-");
    printf("+");
    for (int r = 1; r < h - 1; r++) {
        GotoXY(x, y + r); printf("|");
        GotoXY(x + w - 1, y + r); printf("|");
    }
    GotoXY(x, y + h - 1); printf("+");
    for (int i = 0; i < w - 2; i++) printf("-");
    printf("+");
    SetColor(7);
}

void DrawCell(int r, int c, bool isCursor) {
    int screenX = 4 * c + LEFT + 2;
    int screenY = 2 * r + TOP + 1;
    CellState st = g_game.board[r][c];

    char ch = (st == _X) ? 'X' : (st == _Y ? 'O' : ' ');
    int color = 7;
    if (st == _X) color = WIN_COLORS[g_game.settings.color_P1 % 4];
    else if (st == _Y) color = WIN_COLORS[g_game.settings.color_P2 % 4];

    GotoXY(screenX - 1, screenY);
    if (isCursor) {
        SetColor(14); printf("[");
        SetColor(color); printf("%c", ch);
        SetColor(14); printf("]");
    } else {
        SetColor(color); printf(" %c ", ch);
    }
    SetColor(7);
}

static void DrawBoardGrid(void) {
    SetColor(8);
    for (int j = 0; j <= BOARD_SIZE; j++) {
        for (int i = 0; i <= BOARD_SIZE; i++) {
            GotoXY(LEFT + 4 * i, TOP + 2 * j);
            printf(i < BOARD_SIZE ? "+---" : "+");
            if (j < BOARD_SIZE) {
                GotoXY(LEFT + 4 * i, TOP + 2 * j + 1);
                printf("|");
            }
        }
    }
    SetColor(7);
}

static void DrawPanelFrame(void) {
    int w = 36, h = 24;
    SetColor(11);
    GotoXY(PANEL_X, TOP); printf("+");
    for (int i = 0; i < w - 2; i++) printf("-");
    printf("+");
    for (int r = 1; r < h; r++) {
        GotoXY(PANEL_X, TOP + r); printf("|");
        GotoXY(PANEL_X + w - 1, TOP + r); printf("|");
    }
    GotoXY(PANEL_X, TOP + h); printf("+");
    for (int i = 0; i < w - 2; i++) printf("-");
    printf("+");

    GotoXY(PANEL_X + 1, TOP + 2);  for (int i = 0; i < w - 2; i++) printf("-");
    GotoXY(PANEL_X + 1, TOP + 14); for (int i = 0; i < w - 2; i++) printf("-");

    SetColor(14);
    GotoXY(PANEL_X + 13, TOP + 1); printf("N H O M 1 0");
    SetColor(7);

    GotoXY(PANEL_X + 2, TOP + 15); printf("Phim tat:");
    GotoXY(PANEL_X + 2, TOP + 16); printf("P1: %-6s | P2: %-6s",
           g_game.settings.P1_control == KEYSET_WASD ? "WASD" : "ARROWS",
           g_game.settings.P2_control == KEYSET_WASD ? "WASD" : "ARROWS");
    GotoXY(PANEL_X + 2, TOP + 17); printf("Enter/Space : Danh co");
    GotoXY(PANEL_X + 2, TOP + 18); printf("Z : Undo    | H : Goi y");
    GotoXY(PANEL_X + 2, TOP + 19); printf("L : Luu     | T : Tai game");
    GotoXY(PANEL_X + 2, TOP + 20); printf("R : Choi lai| M : Bat/tat loa");
    GotoXY(PANEL_X + 2, TOP + 21); printf("ESC         : Tam dung");
}

void UpdateTimerDisplay(void) {
    GotoXY(PANEL_X + 2, TOP + 4);
    SetColor(7); printf("Thoi gian: ");
    if (g_game.settings.TurnTimeOut <= 0) {
        SetColor(10); printf("%-20s", "Khong gioi han");
    } else {
        SetColor(g_game.remainTime <= 5.0f ? 12 : 14);
        printf("%-5.1f giay          ", g_game.remainTime);
    }
    SetColor(7);
}

void UpdatePanelStats(void) {
    int cP1 = WIN_COLORS[g_game.settings.color_P1 % 4];
    int cP2 = WIN_COLORS[g_game.settings.color_P2 % 4];

    GotoXY(PANEL_X + 2, TOP + 3);
    SetColor(7); printf("Luot di  : ");
    if (g_game.currentTurn == _X) {
        SetColor(cP1); printf("%-20s", "Nguoi choi 1 (X)");
    } else {
        SetColor(cP2); printf("%-20s", g_game.isVsAI ? "May tinh AI (O)" : "Nguoi choi 2 (O)");
    }

    UpdateTimerDisplay();

    GotoXY(PANEL_X + 2, TOP + 6);
    SetColor(cP1); printf("NGUOI CHOI 1 [X]");
    GotoXY(PANEL_X + 2, TOP + 7);
    SetColor(7); printf("  Che do : %-20s", g_game.isVsAI ? "Dau voi AI" : "2 Nguoi choi");

    GotoXY(PANEL_X + 2, TOP + 9);
    SetColor(cP2); printf("%s", g_game.isVsAI ? "MAY TINH AI [O]" : "NGUOI CHOI 2 [O]");
    GotoXY(PANEL_X + 2, TOP + 10);
    SetColor(7); printf("  Am thanh: %-19s", g_game.settings.SoundEnabled ? "BAT" : "TAT");

    GotoXY(PANEL_X + 2, TOP + 12);
    SetColor(14); printf("Tong so nuoc da di: %-10d", move_count);
    SetColor(7);
}

void DrawGameScreen(void) {
    system("cls");
    DrawBoardGrid();
    DrawPanelFrame();
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (g_game.board[i][j] != EMPTY) DrawCell(i, j, false);
        }
    }
    DrawCell(g_game.CursorY, g_game.CursorX, true);
    UpdatePanelStats();
}

void FlashWinCells(void) {
    if (win_len <= 0) return;
    int color = (g_game.winner == _X) ? WIN_COLORS[g_game.settings.color_P1 % 4] : WIN_COLORS[g_game.settings.color_P2 % 4];
    char ch = (g_game.winner == _X) ? 'X' : 'O';
    for (int t = 0; t < 10; t++) {
        for (int k = 0; k < win_len; k++) {
            int r = win_r[k], c = win_c[k];
            GotoXY(4 * c + LEFT + 1, 2 * r + TOP + 1);
            SetColor(t % 2 == 0 ? (color << 4) : color);
            printf(" %c ", ch);
        }
        PlayBeep(500 + t * 80, 60);
        Sleep(80);
    }
    SetColor(7);
}

void FlashDrawBoard(void) {
    for (int t = 0; t < 6; t++) {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (g_game.board[i][j] == EMPTY) continue;
                char ch = (g_game.board[i][j] == _X) ? 'X' : 'O';
                int orig = (g_game.board[i][j] == _X) ? WIN_COLORS[g_game.settings.color_P1 % 4] : WIN_COLORS[g_game.settings.color_P2 % 4];
                GotoXY(4 * j + LEFT + 1, 2 * i + TOP + 1);
                SetColor(t % 2 == 0 ? 14 : orig);
                printf(" %c ", ch);
            }
        }
        PlayBeep(400, 80);
        Sleep(100);
    }
    SetColor(7);
}

void RenderStateScreen(void) {
    system("cls");
    DrawTitle();

    switch (g_game.state) {
        case STATE_MENU: {
            const char* items[6] = { "Play", "Load Game", "About", "Settings", "Tutorial", "Thoat" };
            DrawBox(34, 10, 36, 14);
            for (int k = 0; k < 6; k++) {
                GotoXY(43, 11 + 2 * k);
                SetColor(k == g_game.menuIndex ? 240 : 7);
                printf("  %-14s  ", items[k]);
            }
            SetColor(8);
            GotoXY(34, 28); printf("W/S hoac Mui ten: Chon | Enter: Xac nhan");
            break;
        }
        case STATE_PLAY_MODE: {
            const char* modes[2] = { "PVP (2 Nguoi choi)", "PVE (Dau voi AI)" };
            DrawBox(32, 10, 40, 9);
            GotoXY(43, 11); SetColor(14); printf("CHON CHE DO CHOI");
            for (int k = 0; k < 2; k++) {
                GotoXY(40, 13 + 2 * k);
                SetColor(k == g_game.menuIndex ? 240 : 7);
                printf("  %-20s  ", modes[k]);
            }
            SetColor(8);
            GotoXY(36, 20); printf("Enter: Bat dau | ESC: Quay lai");
            break;
        }
        case STATE_SETTINGS: {
            const char* colors[] = { "Do", "Xanh Duong", "Xanh La", "Vang" };
            DrawBox(28, 10, 48, 15);
            for (int k = 0; k < 6; k++) {
                GotoXY(32, 12 + 2 * k);
                SetColor(k == g_game.menuIndex ? 240 : 7);
                if (k == 0)      printf(" Sound        : %-18s ", g_game.settings.SoundEnabled ? "ON" : "OFF");
                else if (k == 1) printf(" Phim P1 (X)  : %-18s ", g_game.settings.P1_control == KEYSET_WASD ? "WASD" : "ARROWS");
                else if (k == 2) printf(" Phim P2 (O)  : %-18s ", g_game.settings.P2_control == KEYSET_WASD ? "WASD" : "ARROWS");
                else if (k == 3) printf(" Thoi gian    : %-3d giay           ", g_game.settings.TurnTimeOut);
                else if (k == 4) printf(" Mau P1 (X)   : %-18s ", colors[g_game.settings.color_P1 % 4]);
                else if (k == 5) printf(" Mau P2 (O)   : %-18s ", colors[g_game.settings.color_P2 % 4]);
            }
            SetColor(8);
            GotoXY(28, 26); printf("A/D hoac Trai/Phai: Doi gia tri | ESC: Quay lai");
            break;
        }
        case STATE_ABOUT: {
         DrawBox(24, 10, 56, 13);
            GotoXY(40, 11); SetColor(14); printf("DO AN GAME CARO");
            GotoXY(38, 12); SetColor(11); printf("CSLT - KHTN TP.HCM (K2026)");

            SetColor(7);
            GotoXY(30, 14); printf("1. Phan Duc Anh     - 26120015");
            GotoXY(30, 15); printf("2. Tran Dang Khoa     - 26120187");
            GotoXY(30, 16); printf("3. Nguyen Thanh Tai     - 26120370");
            GotoXY(30, 17); printf("4. Doan Ngoc Huy Hoang     - 26120114");

            GotoXY(36, 20); SetColor(10); printf("Nhan Enter hoac ESC de quay lai...");
            break;
        }
        case STATE_TUTORIAL: {
            DrawBox(22, 10, 60, 13);
            GotoXY(26, 12); SetColor(14); printf("LUAT CHOI: Xep du 5 quan lien tiep de thang.");
            GotoXY(26, 14); SetColor(7);  printf("- Di chuyen : WASD hoac Phim Mui Ten");
            GotoXY(26, 15); printf("- Danh co   : Enter hoac Space");
            GotoXY(26, 16); printf("- Luu / Tai : Phim L (Luu) | Phim T (Tai)");
            GotoXY(26, 17); printf("- Ho tro    : Z (Undo) | H (Goi y) | R (Choi lai)");
            GotoXY(26, 18); printf("- Am thanh  : Phim M (Bat/Tat nhanh)");
            GotoXY(36, 21); SetColor(10); printf("Nhan Enter hoac ESC de quay lai...");
            break;
        }
        case STATE_PAUSE: {
            const char* pItems[3] = { "Tiep tuc", "Choi lai", "Ve Menu chinh" };
            DrawBox(34, 10, 36, 11);
            GotoXY(47, 11); SetColor(14); printf("TAM DUNG");
            for (int k = 0; k < 3; k++) {
                GotoXY(42, 13 + 2 * k);
                SetColor(k == g_game.menuIndex ? 240 : 7);
                printf("  %-16s  ", pItems[k]);
            }
            break;
        }
        case STATE_GAMEOVER: {
            const char* gItems[2] = { "Choi van moi", "Ve Menu chinh" };
            DrawBox(32, 10, 40, 11);
            GotoXY(39, 12);
            if (g_game.winner == _X) {
                SetColor(WIN_COLORS[g_game.settings.color_P1 % 4]);
                printf("NGUOI CHOI 1 (X) CHIEN THANG!");
            } else if (g_game.winner == _Y) {
                SetColor(WIN_COLORS[g_game.settings.color_P2 % 4]);
                printf("%s", g_game.isVsAI ? "MAY TINH AI (O) CHIEN THANG!" : "NGUOI CHOI 2 (O) CHIEN THANG!");
            } else {
                SetColor(14);
                printf("HOA CO!");
            }

            for (int k = 0; k < 2; k++) {
                GotoXY(41, 15 + 2 * k);
                SetColor(k == g_game.menuIndex ? 240 : 7);
                printf("  %-16s  ", gItems[k]);
            }
            break;
        }
        default: break;
    }
    SetColor(7);
}