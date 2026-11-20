#include "event.h"
#include "board.h"
#include "rules.h"
#include "ai.h"
#include <conio.h>
#include <ctype.h>
//Nhan su kien tu phim
RawKey pollRawKey (){
    if(!_kbhit()) return RAW_NONE;
    int ch = _getch();
    if(ch == 0 || ch == 224 || ch == -32){
        ch = _getch();
        if ( ch == 72) return RAW_UP;
        if ( ch == 80) return RAW_DOWN;
        if ( ch == 75) return RAW_LEFT;
        if ( ch == 77) return RAW_RIGHT;
        return RAW_NONE;
    }
    switch (tolower(ch)){
        case 'w': return RAW_W;
        case 's': return RAW_S;
        case 'a':return RAW_A;
        case 'd':return RAW_D;
        case 13:
        case ' ': return RAW_ENTER;
        case 27: return RAW_ESC;
        case 'z':return RAW_Z;
    }
    return RAW_NONE;
}
// Xu ly phim
GameAction mapkeytoAction (RawKey key ){
    if ( key == RAW_NONE) return ACT_NONE;
    if ( key == RAW_ENTER) return ACT_SELECT;
    if ( key == RAW_ESC) return ACT_BACK;
    if ( key == RAW_Z) return ACT_UNDO; 

    if (g_game.state != STATE_PLAYING){
      if( key == RAW_W || key == RAW_UP) return ACT_UP;
      if( key == RAW_S || key == RAW_DOWN) return ACT_DOWN;
      if( key == RAW_A || key == RAW_LEFT) return ACT_LEFT;
      if( key == RAW_D || key == RAW_RIGHT) return ACT_RIGHT;
      return ACT_NONE;
    } 
    //Thay doi control cua player 1, 2
    ControlScheme activeScheme = (g_game.currentTurn == _X) ? g_game.settings.P1_control : g_game.settings.P2_control;
    
    if(activeScheme == KEYSET_WASD){
        if( key == RAW_W ) return ACT_UP;
        if( key == RAW_S ) return ACT_DOWN;
        if( key == RAW_A ) return ACT_LEFT;
        if( key == RAW_D ) return ACT_RIGHT;
    } else{
        if( key == RAW_UP) return ACT_UP;
        if( key == RAW_DOWN) return ACT_DOWN;
        if( key == RAW_LEFT) return ACT_LEFT;
        if( key == RAW_RIGHT) return ACT_RIGHT;
    }
    return ACT_NONE;
}
// xu ly man hinh chinh
static void MainScreen(GameAction act){
    int total = 4;
    if (act == ACT_UP) g_game.menuIndex = (g_game.menuIndex - 1 + total) % total;
    else if( act == ACT_DOWN) g_game.menuIndex = (g_game.menuIndex + 1 ) % total;
    else if ( act == ACT_SELECT){
        int selected = g_game.menuIndex;
        g_game.menuIndex = 0;
        if (selected == 0) g_game.state = STATE_PLAY_MODE;
        else if (selected == 1) g_game.state = STATE_ABOUT;
        else if (selected == 2) g_game.state = STATE_SETTINGS;
        else if (selected == 3) g_game.state = STATE_TUTORIAL;
    }
}
static bool makeMove(int r, int c){
    if (!valid(r,c) || g_game.board[r][c] != EMPTY ) return false;
    place(r,c, g_game.currentTurn);
    GameResult res = check_result(r,c, &g_game.winner);
    if( res == RESULT_WIN || res == RESULT_DRAW) {
        g_game.isWinning = true;
        return true;
    }
    g_game.currentTurn = (g_game.currentTurn == _X) ? _Y : _X;
    return true;
}
static void makeAIMove(void) {
    if (g_game.isWinning) return;
    int botR = 0, botC = 0;
    bot_move(g_game.currentTurn, &botR, &botC);
    if (makeMove(botR, botC)) {
        g_game.CursorY = botR;
        g_game.CursorX = botC;
    }
}
// Dem thoi gian
static void resetTimeCounter(){
    g_game.remainTime = g_game.settings.TurnTimeOut;    
}
void Playing(GameAction act, float dt){
    bool isHumanTurn = (!g_game.isVsAI) || (g_game.currentTurn == _X);
    if (g_game.settings.TurnTimeOut > 0 && isHumanTurn) {
        g_game.remainTime -= dt;
        if(g_game.remainTime <= 0.0f){
            g_game.remainTime = 0.0f;
            g_game.isWinning = true;
            g_game.winner = (g_game.currentTurn == _X) ? _Y : _X;
            g_game.state = STATE_GAMEOVER;
            return;
        }
    }
    if (act == ACT_NONE) return;
    if (act == ACT_UP && g_game.CursorY > 0) g_game.CursorY--;
    else if (act == ACT_DOWN && g_game.CursorY < BOARD_SIZE - 1) g_game.CursorY++;
    else if (act == ACT_LEFT && g_game.CursorX > 0) g_game.CursorX--;
    else if (act == ACT_RIGHT && g_game.CursorX < BOARD_SIZE - 1) g_game.CursorX++;
    if (act == ACT_BACK) {
        g_game.menuIndex = 0;
        g_game.state = STATE_PAUSE;
    }
    if (act == ACT_SELECT){
        bool ValidMove = makeMove(g_game.CursorY, g_game.CursorX);
        if (ValidMove){
            if (g_game.isWinning){
                g_game.state = STATE_GAMEOVER;
                return;
            }
            resetTimeCounter();
            if (g_game.isVsAI){
                makeAIMove();
                if (g_game.isWinning){
                    g_game.state = STATE_GAMEOVER;
                    return;
                }
                resetTimeCounter();
            }
        }
    }
    else if ( act == ACT_UNDO){
        undo_move();
    }
}
// xu ly che do choi
static void PlayMode (GameAction act){
    int total = 2;
    if (act == ACT_UP) g_game.menuIndex = (g_game.menuIndex - 1 + total) % total;
    else if ( act == ACT_DOWN) g_game.menuIndex = (g_game.menuIndex + 1) % total;
    else if ( act == ACT_BACK){
        g_game.state = STATE_MENU;
        g_game.menuIndex = 0;
    }
    else if ( act == ACT_SELECT){
        bool isVsAI = (g_game.menuIndex == 1);
        startNewMatch(isVsAI);
        g_game.remainTime = (float) g_game.settings.TurnTimeOut;
        g_game.state = STATE_PLAYING;
    }
}
// xu ly cai dat
static void SettingsMenu (GameAction act){
    int totalSettings = 6;
    if ( act == ACT_UP) g_game.menuIndex = (g_game.menuIndex - 1 + totalSettings) % totalSettings;
    else if ( act == ACT_DOWN) g_game.menuIndex = (g_game.menuIndex + 1) % totalSettings;
    else if ( act == ACT_BACK){
         g_game.menuIndex = 2;
         g_game.state = STATE_MENU;
    }
    else if ( act == ACT_LEFT || act == ACT_RIGHT || act == ACT_SELECT){
        int dir = (act == ACT_LEFT) ? -1 : 1 ;
        switch(g_game.menuIndex){
            case 0: 
            g_game.settings.SoundEnabled = !g_game.settings.SoundEnabled;
            break;
            case 1:
            g_game.settings.P1_control = (g_game.settings.P1_control == KEYSET_WASD) ? KEYSET_ARROWS : KEYSET_WASD;
            break;
            case 2:
            g_game.settings.P2_control = (g_game.settings.P2_control == KEYSET_ARROWS) ? KEYSET_WASD : KEYSET_ARROWS;
            break;
            case 3:{
            int TimeOptions[] = {15 , 30 , 60, 0};
            int currentIdx = 0;
            int TotalOptions = 4;
            for (int i = 0; i < TotalOptions; i++){
                if (g_game.settings.TurnTimeOut == TimeOptions[i]){
                    currentIdx = i;
                    break;
                }
            }
            currentIdx = (currentIdx + dir + TotalOptions) % TotalOptions;
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
// Xu ly man hinh About, Tutorial
static void InfoScreen (GameAction act, int returnMenuIndex){
    if ( act == ACT_BACK || act == ACT_SELECT){
        g_game.menuIndex = returnMenuIndex;
        g_game.state = STATE_MENU;
    }
}
// Xu ly game pause
static void PauseGame(GameAction act){
  int total = 3;
  if ( act == ACT_UP) g_game.menuIndex = (g_game.menuIndex - 1 + total) % total;
  else if ( act == ACT_DOWN) g_game.menuIndex = (g_game.menuIndex + 1) % total;
  else if ( act == ACT_BACK) g_game.state = STATE_PLAYING;
  else if ( act == ACT_SELECT){
    if ( g_game.menuIndex == 0 ) g_game.state = STATE_PLAYING;
    else if (g_game.menuIndex == 1){
        startNewMatch(g_game.isVsAI);
        g_game.remainTime = (float)g_game.settings.TurnTimeOut;
        g_game.state = STATE_PLAYING;
    }
    else if ( g_game.menuIndex == 2){
        g_game.menuIndex = 0;
        g_game.state = STATE_MENU;
    }
  }
}
// Xu ly game over
static void GameOver (GameAction act ){
    int total = 2;
    if ( act == ACT_UP) g_game.menuIndex = (g_game.menuIndex - 1 + total) % total;
    else if ( act == ACT_DOWN) g_game.menuIndex = (g_game.menuIndex + 1) % total;
    else if ( act == ACT_SELECT){
        if ( g_game.menuIndex == 0){
            startNewMatch(g_game.isVsAI);
            g_game.remainTime = (float)g_game.settings.TurnTimeOut;
            g_game.state = STATE_PLAYING;
        }
        else if ( g_game.menuIndex == 1){
            g_game.menuIndex = 0;
            g_game.state = STATE_MENU;
        }
    }
}
//Tao tran dau
void startNewMatch(bool vsAI) {
    board_reset();
    g_game.currentTurn = _X;
    g_game.CursorX = BOARD_SIZE / 2; // Ô tâm (7, 7)
    g_game.CursorY = BOARD_SIZE / 2;
    g_game.winner = EMPTY;
    g_game.isWinning = false;
    g_game.isVsAI = vsAI;
    g_game.aiSide = _Y;
    g_game.remainTime = (float)g_game.settings.TurnTimeOut;
}
// Luong Game
void dispatchGameFlow(float dt){
    RawKey key = pollRawKey();
    GameAction action = mapkeytoAction(key);
    switch(g_game.state){
        case STATE_MENU: MainScreen(action); break;
        case STATE_PLAY_MODE: PlayMode(action); break;
        case STATE_SETTINGS: SettingsMenu(action); break;
        case STATE_ABOUT: InfoScreen (action, 1); break;
        case STATE_TUTORIAL: InfoScreen (action, 3); break;
        case STATE_PLAYING: Playing (action, dt); break;
        case STATE_PAUSE: PauseGame (action); break;
        case STATE_GAMEOVER: GameOver (action); break;
        default: break;
    }
}