#ifndef LADX_BANK3_ENTITIES_LIFTABLE_ROCK_H
#define LADX_BANK3_ENTITIES_LIFTABLE_ROCK_H

#include "gb.h"

/* Liftable Rock Entity Handlers (03:5326-03:5406) */
void Entity4BHandler(GBState *gb, uint16_t bc);
void LiftableRockEntityHandler(GBState *gb, uint16_t bc);
void LiftableRockIntactHandler(GBState *gb, uint16_t bc);
void LiftableRockStartSmashingAnimation(GBState *gb, uint16_t bc);

/* Smash Rock (03:5407) */
void SmashRock(GBState *gb, uint16_t bc);

#endif /* LADX_BANK3_ENTITIES_LIFTABLE_ROCK_H */