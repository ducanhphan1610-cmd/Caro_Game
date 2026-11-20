#ifndef EVENT_H
#define EVENT_H
#include "caro.h"
typedef enum{
    RAW_NONE = 0,
    RAW_W, RAW_A, RAW_S, RAW_D,
    RAW_UP, RAW_LEFT, RAW_DOWN, RAW_RIGHT,
    RAW_ENTER,
    RAW_ESC,
    RAW_Z,
} RawKey;

typedef enum{
    ACT_NONE = 0,
    ACT_UP, ACT_DOWN, ACT_LEFT, ACT_RIGHT,
    ACT_SELECT,
    ACT_BACK,
    ACT_UNDO,
} GameAction;

RawKey pollRawKey ();
GameAction mapkeytoAction ( RawKey key);
void dispatchGameFlow(float dt);
void startNewMatch(bool vsAI);
void Playing(GameAction act, float dt);
#endif