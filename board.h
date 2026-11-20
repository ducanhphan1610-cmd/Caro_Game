#ifndef BOARD_H        
#define BOARD_H
#include "caro.h"
#include <stdbool.h>
  
extern int  hist_r[BOARD_SIZE * BOARD_SIZE];         
extern int  hist_c[BOARD_SIZE * BOARD_SIZE];
extern int  move_count;                              
extern int  win_r[BOARD_SIZE];                       
extern int  win_c[BOARD_SIZE];
extern int  win_len;                                 
extern bool block_rule;                              

bool valid(int r, int c);                 
void board_reset(void);                   
void place(int r, int c, CellState player);    
bool undo_move(void);                     

#endif
