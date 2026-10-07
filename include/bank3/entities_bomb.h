#ifndef LADX_BANK3_ENTITIES_BOMB_H
#define LADX_BANK3_ENTITIES_BOMB_H

#include "gb.h"

/* Bomb Entity Handlers (03:65E2-03:68F0) */
void BombExplosionHandler(GBState *gb, uint16_t bc);
void BombExplosionVisuals(GBState *gb, uint16_t bc);
void RenderBombExplosion(GBState *gb, uint16_t bc);
void BombEntityHandler(GBState *gb, uint16_t bc);
void RenderBomb(GBState *gb, uint16_t bc);
void CheckForBombDestroyableObjectPuzzle(GBState *gb, uint16_t bc);
void CheckForBombDestroyableObjectBasic(GBState *gb, uint16_t bc);
void CheckExplosionInteractionWithEntities(GBState *gb, uint16_t bc);
void BombBounceOffWalls(GBState *gb, uint16_t bc);

#endif /* LADX_BANK3_ENTITIES_BOMB_H */