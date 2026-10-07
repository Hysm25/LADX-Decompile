#ifndef LADX_BANK3_ENTITIES_PUSHED_BLOCK_H
#define LADX_BANK3_ENTITIES_PUSHED_BLOCK_H

#include "gb.h"

/* Pushed Block Entity Handler (03:5249) */
void PushedBlockEntityHandler(GBState *gb, uint16_t bc);

/* func_003_52D4 (03:52D4) - Helper for pushed blocks */
void func_003_52D4(GBState *gb, uint16_t bc);

/* func_003_51C9 (03:51C9) - Tile replacement and draw command helper for pushed blocks */
void func_003_51C9(GBState *gb, uint16_t bc, const uint8_t *data_ptr, uint8_t b_val);

#endif /* LADX_BANK3_ENTITIES_PUSHED_BLOCK_H */