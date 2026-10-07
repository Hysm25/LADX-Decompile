#ifndef LADX_BANK3_ENTITIES_PHYSICS_H
#define LADX_BANK3_ENTITIES_PHYSICS_H

#include "gb.h"

/* Physics and Collision Helpers (03:6B7B-03:6BC5, 03:60B3-03:6156) */

/* func_003_6B7B (03:6B7B) - Apply gravity and underwater physics for entities */
void func_003_6B7B(GBState *gb, uint16_t bc);

/* BouncingEntityPhysics (03:60B3) - Handles entity bouncing physics */
void BouncingEntityPhysics(GBState *gb, uint16_t bc);

/* func_003_6C6B (03:6C6B) - Frame-based collision check for enemy damage */
bool func_003_6C6B(GBState *gb, uint16_t bc);

/* func_003_6CC0 (03:6CC0) - Harmless entity check for collision detection */
bool func_003_6CC0(GBState *gb, uint16_t bc);

/* func_003_6DDF (03:6DDF) - Enemy damage reactions */
void func_003_6DDF(GBState *gb, uint16_t bc);

/* func_003_6F5C (03:6F5C) - ConfigureEntityRecoil wrapper */
void func_003_6F5C(GBState *gb, uint16_t bc);

/* func_003_6F93 (03:6F93) - JINGLE_BUMP and recoil setup */
void func_003_6F93(GBState *gb);

/* label_003_6FA7 (03:6FA7) - Apply horizontal bump velocity to Link */
void label_003_6FA7(GBState *gb, uint8_t magnitude);

/* func_003_7565 (03:7565) - Push Link away from entity */
void func_003_7565(GBState *gb);
void func_003_7565_with_length(GBState *gb, uint8_t length);

/* func_003_75A2 (03:75A2) - Entity collision detection with other entities */
void func_003_75A2(GBState *gb, uint16_t bc);

/* Tables for background and entity interaction */
extern const int8_t EntityCollisionPointsX[16];
extern const int8_t EntityCollisionPointsY[16];
extern const uint8_t CollisionsTableFlagPerDirection[4];
extern const int8_t EntityOnConveyorMovementX[8];
extern const int8_t EntityOnConveyorMovementY[8];
extern const uint8_t FineCollisionShapes[72];
extern const uint8_t SwitchBlockLoweredStatePerObject[2];

/* ApplyEntityCollisionWithObject (03:7ACD) */
bool ApplyEntityCollisionWithObject(GBState *gb, uint16_t bc, uint16_t de);

/* ApplyEntityInteractionWithBackground (03:7386) */
void ApplyEntityInteractionWithBackground(GBState *gb, uint16_t bc);

/* ApplySwordIntersectionWithObjects (03:8194) */
void ApplySwordIntersectionWithObjects(GBState *gb, uint16_t bc);

/* GetVectorTowardsLink (03:8508) */
void GetVectorTowardsLink(GBState *gb, uint8_t *x, uint8_t *y);
void GetVectorTowardsLink_with_length(GBState *gb, uint8_t length, uint8_t *val0, uint8_t *val1);

/* ApplyVectorTowardsLink (03:7EC7) */
void ApplyVectorTowardsLink(GBState *gb, uint16_t bc);

/* GetEntityDirectionToLink_03 (03:8691) - Returns direction to Link */
uint8_t GetEntityDirectionToLink_03(GBState *gb);

/* GetEntityXDistanceToLink_03 (03:8647) */
void GetEntityXDistanceToLink_03(GBState *gb, uint8_t *e, uint8_t *d);
void GetEntityXDistanceToLink_03_idx(GBState *gb, uint16_t bc, uint8_t *e, uint8_t *d);

/* GetEntityYDistanceToLink_03 (03:8668) */
void GetEntityYDistanceToLink_03(GBState *gb, uint8_t *e, uint8_t *d);
void GetEntityYDistanceToLink_03_idx(GBState *gb, uint16_t bc, uint8_t *e, uint8_t *d);

/* UpdateEntityPosWithSpeed_03 (03:8729) */
void UpdateEntityPosWithSpeed_03(GBState *gb, uint16_t bc);

/* AddEntitySpeedToPos_03 (03:8748) */
void AddEntitySpeedToPos_03(GBState *gb, uint16_t bc);

/* AddEntityZSpeedToPos_03 (03:8790) */
void AddEntityZSpeedToPos_03(GBState *gb, uint16_t bc);

/* ReturnIfNonInteractive_03 (03:8810) */
bool ReturnIfNonInteractive_03(GBState *gb, bool allowInactiveEntity);

/* ApplyRecoilIfNeeded_03 (03:8850) */
void ApplyRecoilIfNeeded_03(GBState *gb, uint16_t bc);

/* ConfigureEntityRecoil (03:6FCC) */
void ConfigureEntityRecoil(GBState *gb, uint16_t bc, uint8_t recoil_amount);

/* StartIgnoringHitsForEntity (03:73DB) */
void StartIgnoringHitsForEntity(GBState *gb);
void StartIgnoringHitsForEntity_idx(GBState *gb, uint16_t bc);

/* Helper Functions (03:4F12+) */
void func_003_4F12(GBState *gb, uint16_t bc);
void SetHiddenDroppableOptions1(GBState *gb, uint16_t bc);
void EntityShiftPosition(GBState *gb, uint16_t bc);
void EntityShiftPosition_shiftBy8(GBState *gb, uint16_t bc, uint16_t sign_table, uint16_t pos_table);

/* Trampoline/Callback Stubs (declared in home/entities.h) */
void ResetPegasusBoots(GBState *gb);
void IncrementEntityState(GBState *gb, uint16_t bc);
void ClearEntitySpeed(GBState *gb, uint16_t bc);
void UnloadEntity(GBState *gb, uint16_t bc);
void UnloadEntityAndReturn(GBState *gb, uint16_t bc);
void ConfigureEntityHitbox(GBState *gb, uint16_t bc);
void label_27F2(GBState *gb);
void AlertSwordMoblins(GBState *gb);
void PlayBombExplosionSfx(GBState *gb);
void MarkTriggerAsResolved(GBState *gb);
void CopyLinkFinalPositionToActivePosition(GBState *gb);
void CopyEntityPositionToActivePosition(GBState *gb, uint16_t bc);
void func_003_51C9(GBState *gb, uint16_t entity_index, const uint8_t *data_ptr, uint8_t b_val);
void func_003_52D4(GBState *gb, uint16_t bc);
void func_003_61C0(GBState *gb, uint16_t bc);
void func_003_7E0E(GBState *gb, uint16_t bc);
void func_003_6E2B(GBState *gb, uint16_t bc);
void func_003_4E16(GBState *gb, uint16_t bc);
void OpenDialogInTable0_trampoline(GBState *gb, uint8_t dialog_id);

#endif /* LADX_BANK3_ENTITIES_PHYSICS_H */