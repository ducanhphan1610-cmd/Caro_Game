#include"caro.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

int SaveGame (const char *filename){
    FILE *f = fopen(filename, "w");
    if( f == NULL) return 0;
    fprintf(f,"%s\n",g_game.gamename);
    fprintf(f,"%d\n",g_game.currentTurn);
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            fprintf(f,"%d", g_game.board[i][j]);
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
    if(fscanf(f , "%[^\n]",g_game.gamename) != 1){
        fclose(f);
        return 0;
    }
    if(fscanf(f, "%d", g_game.currentTurn) != 1){
        fclose(f);
        return 0;
    }
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0 ; j < BOARD_SIZE; j++){
            if(fscanf(f, "%ld", g_game.board[i][j]) != 1){
               fclose(f);
               return 0;
            }
    }
    fclose(f);
    return 1;   
}