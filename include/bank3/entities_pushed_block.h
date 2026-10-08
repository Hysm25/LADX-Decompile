#ifndef LADX_BANK3_ENTITIES_PUSHED_BLOCK_H
#define LADX_BANK3_ENTITIES_PUSHED_BLOCK_H

#include "gb.h"

/* Pushed Block Data Tables (03:5156-03:5245) */
extern const uint8_t Data_003_5156[4];
extern const uint8_t Data_003_515A[4];
extern const uint8_t Data_003_5166[4];
extern const uint8_t Data_003_516A[4];
extern const uint8_t Data_003_523D[4];
extern const uint8_t Data_003_5241[4];

/* EntityInitPushedBlock (03:516E) */
void EntityInitPushedBlock(GBState *gb, uint16_t bc);

/* Pushed Block Entity Handler (03:5249) */
void PushedBlockEntityHandler(GBState *gb, uint16_t bc);

/* func_003_52D4 (03:52D4) - Helper for pushed blocks */
void func_003_52D4(GBState *gb, uint16_t bc);

/* func_003_51C9 (03:51C9) - Tile replacement and draw command helper for pushed blocks */
void func_003_51C9(GBState *gb, uint16_t bc, const uint8_t *data_ptr, uint8_t b_val);

/* label_003_51F5 (03:51F5) - Appends 2x2 tile draw command for intersected object */
void label_003_51F5(GBState *gb, const uint8_t *data_ptr);

/* MarkRoomCompleted (03:512A) */
void MarkRoomCompleted(GBState *gb);

/* GetRoomStatusAddressInHL (03:5134) */
uint16_t GetRoomStatusAddressInHL(GBState *gb);

#endif /* LADX_BANK3_ENTITIES_PUSHED_BLOCK_H */