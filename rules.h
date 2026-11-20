#ifndef RULES_H
#define RULES_H
#include "caro.h"
#include <stdbool.h>

typedef enum {
    RESULT_CONTINUE,  
    RESULT_WIN,        
    RESULT_DRAW        
} GameResult;


bool check_win(int r, int c);


bool check_draw(void);


GameResult check_result(int r, int c, int *winner);

#endif