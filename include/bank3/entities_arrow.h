#ifndef LADX_BANK3_ENTITIES_ARROW_H
#define LADX_BANK3_ENTITIES_ARROW_H

#include "gb.h"

/* Arrow Entity Handlers (03:6A34-03:6B71) */
void ArrowEntityHandler(GBState *gb, uint16_t bc);
void BombArrowHandler(GBState *gb, uint16_t bc);
void MoblinArrowEntityHandler(GBState *gb, uint16_t bc);
void ArrowRenderAndMove(GBState *gb, uint16_t bc);
void ArrowRenderAndMove_skipRendering(GBState *gb, uint16_t bc);
void EntityBounceOffWallX(GBState *gb, uint16_t bc);
void EntityBounceOffWallY(GBState *gb, uint16_t bc);
void ArrowRockAfterHittingWall(GBState *gb, uint16_t bc);

/* Octorok Rock sprite variants (03:6A1E) */
extern const uint8_t OctorokRockSpriteVariants[8];

/* Octorok Rock Entity Handler (03:6A26) */
void OctorokRockEntityHandler(GBState *gb, uint16_t bc);

/* Octorok Entity Handler (03:57E9) */
void OctorokEntityHandler(GBState *gb, uint16_t bc);

#endif /* LADX_BANK3_ENTITIES_ARROW_H */