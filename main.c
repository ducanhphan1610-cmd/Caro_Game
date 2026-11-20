#include"caro.h"
#include "event.h"
#include "board.h"
#include "rules.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <time.h>

Game g_game;

int SaveGame (const char *filename){
    FILE *f = fopen(filename, "w");
    if( f == NULL) return 0;
    fprintf(f,"%s\n",g_game.gamename);
    fprintf(f,"%d\n",g_game.currentTurn, g_game.isVsAI ? 1 : 0);
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            fprintf(f,"%d ", g_game.board[i][j]);
        }
        fprintf(f,"\n");
    }
    fclose(f);
    return 1;
}

int LoadGame (const char *filename){
    FILE *f = fopen(filename,"r");
    if ( f == NULL){
        printf_s("Tệp không tồn tại!\n");
        return 0;
    }
    if(fscanf(f , " %[^\n]",g_game.gamename) != 1){
        fclose(f);
        return 0;
    }
    int turn = 1, vsAI = 0;
    if (fscanf(f, "%d %d", &turn, &vsAI) != 2){
        fclose(f);
        return 0;
    }
    g_game.currentTurn = (CellState) turn;
    g_game.isVsAI = (vsAI == 1);
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0 ; j < BOARD_SIZE; j++){
            int cell = 0;
            if(fscanf(f, " %d", &cell) != 1){
               fclose(f);
               return 0;
            }
            g_game.board[i][j] = (CellState)cell;
        }
    }
    fclose(f);
    return 1;   
}
void renderConsole(void) {
    system("cls");
    switch (g_game.state) {
        case STATE_MENU: {
            const char *items[] = {"Play", "About", "Settings", "Tutorial"};
            printf("=== %s - MAIN MENU ===\n\n", g_game.gamename);
            for (int i = 0; i < 4; i++) {
                printf("%s %s\n", (i == g_game.menuIndex) ? "-->" : "   ", items[i]);
            }
            printf("\n[W/S hoặc Mui ten: Di chuyen | Enter: Chon]");
            break;
        }
        case STATE_PLAY_MODE: {
            const char *modes[] = {"PVP (2 Nguoi choi)", "PVE (Dau voi AI)"};
            printf("=== CHON CHE DO CHOI ===\n\n");
            for (int i = 0; i < 2; i++) {
                printf("%s %s\n", (i == g_game.menuIndex) ? "-->" : "   ", modes[i]);
            }
            printf("\n[Enter: Bat dau | ESC: Quay lai]");
            break;
        }
        case STATE_SETTINGS: {
            const char *colors[] = {"Do", "Xanh Duong", "Xanh La", "Vang"};
            printf("=== SETTINGS (Dung A/D hoac Trai/Phai de doi gia tri) ===\n\n");
            printf("%s Sound:       <%s>\n", g_game.menuIndex == 0 ? "-->" : "   ", g_game.settings.SoundEnabled ? "ON" : "OFF");
            printf("%s Phim P1 (X): <%s>\n", g_game.menuIndex == 1 ? "-->" : "   ", g_game.settings.P1_control == KEYSET_WASD ? "WASD" : "ARROWS");
            printf("%s Phim P2 (Y): <%s>\n", g_game.menuIndex == 2 ? "-->" : "   ", g_game.settings.P2_control == KEYSET_WASD ? "WASD" : "ARROWS");
            printf("%s Thoi gian:   <%d giay>\n", g_game.menuIndex == 3 ? "-->" : "   ", g_game.settings.TurnTimeOut);
            printf("%s Mau P1 (X):  <%s>\n", g_game.menuIndex == 4 ? "-->" : "   ", colors[g_game.settings.color_P1]);
            printf("%s Mau P2 (Y):  <%s>\n", g_game.menuIndex == 5 ? "-->" : "   ", colors[g_game.settings.color_P2]);
            printf("\n[ESC: Luu & Quay lai]");
            break;
        }
        case STATE_ABOUT:
            printf("=== ABOUT ===\nDo an Game Caro - Nhom Phat Trien\n\n[Bam Enter/ESC de quay lai]");
            break;
        case STATE_TUTORIAL:
            printf("=== TUTORIAL ===\n- Xep du 5 quan lien tiep de thang (Co luat chan 2 dau).\n- Phim Z: Undo | ESC: Tam dung.\n\n[Bam Enter/ESC de quay lai]");
            break;
        case STATE_PLAYING: {
            printf("Luot: %c | Thoi gian con lai: %.1fs | [Z: Undo | ESC: Pause]\n\n",
                   (g_game.currentTurn == _X) ? 'X' : 'Y', g_game.remainTime);
            for (int i = 0; i < BOARD_SIZE; i++) {
                for (int j = 0; j < BOARD_SIZE; j++) {
                    char ch = '.';
                    if (g_game.board[i][j] == _X) ch = 'X';
                    else if (g_game.board[i][j] == _Y) ch = 'Y';

                    if (i == g_game.CursorY && j == g_game.CursorX) printf("[%c]", ch);
                    else printf(" %c ", ch);
                }
                printf("\n");
            }
            break;
        }
        case STATE_PAUSE: {
            const char *pItems[] = {"Tiep tuc", "Choi lai", "Ve Menu chinh"};
            printf("=== TAM DUNG ===\n\n");
            for (int i = 0; i < 3; i++) {
                printf("%s %s\n", (i == g_game.menuIndex) ? "-->" : "   ", pItems[i]);
            }
            break;
        }
        case STATE_GAMEOVER: {
            const char *gItems[] = {"Choi van moi", "Ve Menu chinh"};
            printf("=== KET THUC VAN DAU ===\n");
            if (g_game.winner == _X) printf("NGUOI CHOI X THANG!\n\n");
            else if (g_game.winner == _Y) printf("NGUOI CHOI Y (AI) THANG!\n\n");
            else printf("HOA CO!\n\n");

            for (int i = 0; i < 2; i++) {
                printf("%s %s\n", (i == g_game.menuIndex) ? "-->" : "   ", gItems[i]);
            }
            break;
        }
        default: break;
    }
}
int main(){
    srand((unsigned int)time(NULL));
    strcpy(g_game.gamename, "CARO 15x15");
    g_game.settings.SoundEnabled = true;
    g_game.settings.TurnTimeOut = 30;
    g_game.settings.P1_control = KEYSET_WASD;
    g_game.settings.P2_control = KEYSET_ARROWS;
    g_game.state = STATE_MENU;
    
    clock_t lastTime = clock();
    float renderTimer = 0.0f;

    renderConsole();
    
    while (g_game.state != STATE_QUIT) {
        clock_t now = clock();
        float dt = (float)(now - lastTime) / CLOCKS_PER_SEC;
        lastTime = now;

        GameState prevState = g_game.state;
        int prevMenu = g_game.menuIndex;
        int prevX = g_game.CursorX, prevY = g_game.CursorY;
        int prevMoves = move_count;

        dispatchGameFlow(dt);

        renderTimer += dt;
        bool stateChanged = (prevState != g_game.state) || (prevMenu != g_game.menuIndex) || (prevX != g_game.CursorX) || (prevY != g_game.CursorY) || (prevMoves != move_count);

        if (stateChanged || (g_game.state == STATE_PLAYING && renderTimer >= 0.5f)) {
            renderConsole();
            renderTimer = 0.0f;
        }
    }
    return 0;
}