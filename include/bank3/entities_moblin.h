#ifndef LADX_BANK3_ENTITIES_MOBLIN_H
#define LADX_BANK3_ENTITIES_MOBLIN_H

#include <stdint.h>
#include "gb.h"

/* Data Tables */
extern const uint8_t OctorokSpriteVariants[32];
extern const int8_t RoamingEnemySpeedXPerDirection[4];
extern const int8_t RoamingEnemySpeedYPerDirection[4];
extern const uint8_t EntityVariantForDirection_03[4];
extern const uint8_t MoblinSpriteVariants[32];
extern const int8_t MoblinArrowOffsetXPerDirection[4];
extern const int8_t MoblinArrowOffsetYPerDirection[4];
extern const int8_t MoblinArrowSpeedXPerDirection[4];
extern const int8_t MoblinArrowSpeedYPerDirection[4];
extern const int8_t OctorokRockOffsetXPerDirection[4];
extern const int8_t OctorokRockOffsetYPerDirection[4];
extern const int8_t OctorokRockSpeedXPerDirection[4];
extern const int8_t OctorokRockSpeedYPerDirection[4];
extern const uint8_t MaskedIronMaskSpriteVariants[32];

/* Roaming Enemy & Projectile Functions (03:57E9-03:59D6) */
void OctorokEntityHandler(GBState *gb, uint16_t bc);
void MoblinEntityHandler(GBState *gb, uint16_t bc);
void AnimateRoamingEnemy(GBState *gb, uint16_t bc);
void AnimateRoamingEnemy_with_sprites(GBState *gb, uint16_t bc, const uint8_t *sprites);
void RoamingEnemyState0Handler(GBState *gb, uint16_t bc);
void SetEntityVariantForDirection_03(GBState *gb, uint16_t bc);
void SpawnMoblinArrow(GBState *gb, uint16_t bc);
void SpawnOctorokRock(GBState *gb, uint16_t bc);

#endif /* LADX_BANK3_ENTITIES_MOBLIN_H */