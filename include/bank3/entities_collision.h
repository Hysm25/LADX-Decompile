#ifndef LADX_BANK3_ENTITIES_COLLISION_H
#define LADX_BANK3_ENTITIES_COLLISION_H

#include "gb.h"

/* Collision and Damage Handlers (03:6C72-03:73E6) */
extern const uint8_t EntityDamagesForGroup[53];
extern const uint8_t Data_003_6FE4[4];
extern const uint8_t Data_003_43EC[848];
extern const uint8_t Data_003_473C[128];
extern const uint8_t Data_003_73E7[4];
bool CheckLinkCollisionWithEnemy(GBState *gb, uint16_t bc);
void ApplyLinkCollisionWithEnemy(GBState *gb, uint16_t bc);
void DefaultEnemyDamageCollisionHandler(GBState *gb, uint16_t bc);
void func_003_6E2B(GBState *gb, uint16_t bc);
void EnemyCollidedWithSword(GBState *gb, uint16_t bc);
void ApplySwordDamagesToEnemy(GBState *gb, uint16_t bc);

/* func_003_6B7B is declared in entities_physics.h */
void func_003_6B7B(GBState *gb, uint16_t bc);

/* Enemy Collision Handler for Link (03:73EB-03:74E0, 03:74EC-03:7598) */
extern const uint8_t Data_003_74E4[4];
extern const uint8_t Data_003_74E8[4];
void func_003_73EB(GBState *gb, uint16_t bc);
void label_003_74EC(GBState *gb, uint16_t bc);

/* GetEntityDirectionToLink_03 (03:8691) - Returns direction to Link */
uint8_t GetEntityDirectionToLink_03(GBState *gb);

/* GetEntityXDistanceToLink_03 (03:8647) */
void GetEntityXDistanceToLink_03(GBState *gb, uint8_t *e, uint8_t *d);

/* GetEntityYDistanceToLink_03 (03:8668) */
void GetEntityYDistanceToLink_03(GBState *gb, uint8_t *e, uint8_t *d);

/* UpdateEntityPosWithSpeed_03 (03:8729) */
void UpdateEntityPosWithSpeed_03(GBState *gb, uint16_t bc);

/* AddEntitySpeedToPos_03 (03:8748) */
void AddEntitySpeedToPos_03(GBState *gb, uint16_t bc);

/* AddEntityZSpeedToPos_03 (03:8790) */
void AddEntityZSpeedToPos_03(GBState *gb, uint16_t bc);

/* StartIgnoringHitsForEntity (03:73DB) */
void StartIgnoringHitsForEntity(GBState *gb);
void StartIgnoringHitsForEntity_idx(GBState *gb, uint16_t bc);

/* ResetPegasusBoots (03:0CB6) */
void ResetPegasusBoots(GBState *gb);

/* func_003_7565 (03:7565) */
void func_003_7565(GBState *gb);
void func_003_7565_with_length(GBState *gb, uint8_t length);

#endif /* LADX_BANK3_ENTITIES_COLLISION_H */