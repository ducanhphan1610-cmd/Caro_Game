#ifndef UI_H
#define UI_H

#include "caro.h"

#define LEFT 3
#define TOP 1
#define PANEL_X (LEFT + 4 * BOARD_SIZE + 6)

void SetColor(int pColor);
void SetCursorVisible(bool pVisible);
void FixConsoleWindow(void);
void GotoXY(int x, int y);
void PlayBeep(int pFreq, int pMs);
void ClearLine(int y);
void ShowMsg(int pRow, int pColor, const char* pText);

void InitConsoleUI(void);
void DrawGameScreen(void);
void DrawCell(int r, int c, bool isCursor);
void UpdatePanelStats(void);
void UpdateTimerDisplay(void);
void FlashWinCells(void);
void FlashDrawBoard(void);
void RenderStateScreen(void);

#endif