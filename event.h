#ifndef EVENT_H
#define EVENT_H
#include "caro.h"

typedef enum {
    RAW_NONE = 0,
    RAW_W, RAW_A, RAW_S, RAW_D,
    RAW_UP, RAW_LEFT, RAW_DOWN, RAW_RIGHT,
    RAW_ENTER,
    RAW_ESC,
    RAW_Z,
    RAW_L,
    RAW_T,
    RAW_H,
    RAW_R,
    RAW_M
} RawKey;

typedef enum {
    ACT_NONE = 0,
    ACT_UP, ACT_DOWN, ACT_LEFT, ACT_RIGHT,
    ACT_SELECT,
    ACT_BACK,
    ACT_UNDO,
    ACT_SAVE,
    ACT_LOAD,
    ACT_HINT,
    ACT_RESTART,
    ACT_MUTE
} GameAction;

RawKey pollRawKey(void);
GameAction mapkeytoAction(RawKey key);
void dispatchGameFlow(float dt);
void startNewMatch(bool vsAI);
void Playing(GameAction act, float dt);

bool PromptSaveGame(int msgRow);
bool PromptLoadGame(int msgRow);

#endif