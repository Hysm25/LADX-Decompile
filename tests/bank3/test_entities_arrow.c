#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "gb.h"
#include "bank3/entities_arrow.h"
#include "bank3/entities_physics.h"
#include "bank3/entities_collision.h"
#include "bank3/entities_droppable.h"
#include "constants/entities.h"
#include "constants/memory.h"
#include "constants/directions.h"
#include "constants/gameplay.h"
#include "constants/inventory.h"
#include "constants/sfx.h"
#include "constants/gfx.h"
#include "constants/audio.h"

/* ===== 1. Data Tables Verification ===== */
static void test_DataTables_Arrow(void) {
    printf("[RUN ] DataTables_Arrow\n");

    /* EntityArrowSpriteVariants (16 bytes: 4 variants * 4 bytes) */
    assert(sizeof(EntityArrowSpriteVariants) == 16);
    /* variant 0 (right) */
    assert(EntityArrowSpriteVariants[0] == 0x2E);
    assert(EntityArrowSpriteVariants[1] == (OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP));
    assert(EntityArrowSpriteVariants[2] == 0x2C);
    assert(EntityArrowSpriteVariants[3] == (OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP));
    /* variant 1 (left) */
    assert(EntityArrowSpriteVariants[4] == 0x2C);
    assert(EntityArrowSpriteVariants[5] == (OAM_GBC_PAL_1 | OAMF_PAL0));
    assert(EntityArrowSpriteVariants[6] == 0x2E);
    assert(EntityArrowSpriteVariants[7] == (OAM_GBC_PAL_1 | OAMF_PAL0));
    /* variant 2 (up) */
    assert(EntityArrowSpriteVariants[8] == 0x2A);
    assert(EntityArrowSpriteVariants[9] == (OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_YFLIP));
    assert(EntityArrowSpriteVariants[10] == 0x2A);
    assert(EntityArrowSpriteVariants[11] == (OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_YFLIP | OAMF_XFLIP));
    /* variant 3 (down) */
    assert(EntityArrowSpriteVariants[12] == 0x2A);
    assert(EntityArrowSpriteVariants[13] == (OAM_GBC_PAL_1 | OAMF_PAL0));
    assert(EntityArrowSpriteVariants[14] == 0x2A);
    assert(EntityArrowSpriteVariants[15] == (OAM_GBC_PAL_1 | OAMF_PAL0 | OAMF_XFLIP));

    /* BombArrowBombSprite (2 bytes) */
    assert(sizeof(BombArrowBombSprite) == 2);
    assert(BombArrowBombSprite[0] == 0x80);
    assert(BombArrowBombSprite[1] == (OAM_GBC_PAL_5 | OAMF_PAL1));

    /* BombArrowBomb offsets */
    assert(sizeof(BombArrowBombXOffsetPerDirection) == 4);
    assert(BombArrowBombXOffsetPerDirection[0] == 4);
    assert(BombArrowBombXOffsetPerDirection[1] == -4);
    assert(BombArrowBombXOffsetPerDirection[2] == 0);
    assert(BombArrowBombXOffsetPerDirection[3] == 0);

    assert(sizeof(BombArrowBombYOffsetPerDirection) == 4);
    assert(BombArrowBombYOffsetPerDirection[0] == -2);
    assert(BombArrowBombYOffsetPerDirection[1] == -2);
    assert(BombArrowBombYOffsetPerDirection[2] == -6);
    assert(BombArrowBombYOffsetPerDirection[3] == 4);

    /* ArrowSpinningSpriteVariantFrames */
    assert(sizeof(ArrowSpinningSpriteVariantFrames) == 4);
    assert(ArrowSpinningSpriteVariantFrames[0] == DIRECTION_RIGHT);
    assert(ArrowSpinningSpriteVariantFrames[1] == DIRECTION_DOWN);
    assert(ArrowSpinningSpriteVariantFrames[2] == DIRECTION_LEFT);
    assert(ArrowSpinningSpriteVariantFrames[3] == DIRECTION_UP);

    printf("[PASS] DataTables_Arrow\n");
}

static void setup_interactive(GBState *gb) {
    gb_write_hram(gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(gb, wTransitionSequenceCounter, 0x04);
    gb_write(gb, wDialogState, 0x00);
    gb_write(gb, wC1A8, 0x00);
    gb_write(gb, wInventoryAppearing, 0x00);
    gb_write(gb, wRoomTransitionState, 0x00);
}

/* ===== 2. ArrowEntityHandler Verification ===== */
static void test_ArrowEntityHandler(void) {
    printf("[RUN ] ArrowEntityHandler\n");

    GBState gb;
    gb_init(&gb);
    setup_interactive(&gb);
    uint16_t bc = 0x03;

    /* Setup entity slot */
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);

    /* Test 1: Projectile count increment */
    gb_write(&gb, wActiveProjectileCount, 5);
    gb_write_hram(&gb, hActiveEntityState, 0x01); /* BombArrow branch */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x10);
    ArrowEntityHandler(&gb, bc);
    assert(gb_read(&gb, wActiveProjectileCount) == 6);

    /* Test 2: Transition countdown != 0 bypasses damage type / statue */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityState, 0x00);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x08);
    gb_write(&gb, wAttackDamageType, 0x00);
    gb_write_hram(&gb, hActiveEntitySpriteVariant, DIRECTION_UP);
    gb_write(&gb, wRoomEvent, TRIGGER_SHOOT_STATUE_EYE);
    gb_write_hram(&gb, hObjectUnderEntity, OBJECT_ONE_EYED_STATUE);
    gb_write(&gb, wRoomEventEffectExecuted, 0);

    ArrowEntityHandler(&gb, bc);
    /* wAttackDamageType should NOT be set because countdown != 0 */
    assert(gb_read(&gb, wAttackDamageType) == 0x00);
    /* Trigger should NOT be resolved */
    assert(gb_read(&gb, wRoomEventEffectExecuted) == 0);

    /* Test 3: Normal state 0, countdown 0 sets damage type */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityState, 0x00);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    gb_write(&gb, wAttackDamageType, 0x00);
    /* Variant not UP -> statue trigger check fails */
    gb_write_hram(&gb, hActiveEntitySpriteVariant, DIRECTION_RIGHT);
    gb_write(&gb, wRoomEvent, TRIGGER_SHOOT_STATUE_EYE);
    gb_write_hram(&gb, hObjectUnderEntity, OBJECT_ONE_EYED_STATUE);
    gb_write(&gb, wRoomEventEffectExecuted, 0);

    ArrowEntityHandler(&gb, bc);
    assert(gb_read(&gb, wAttackDamageType) == DAMAGE_TYPE_ARROW);
    assert(gb_read(&gb, wRoomEventEffectExecuted) == 0);

    /* Test 4: Statue eye trigger success */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityState, 0x00);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    gb_write_hram(&gb, hActiveEntitySpriteVariant, DIRECTION_UP);
    gb_write(&gb, wRoomEvent, TRIGGER_SHOOT_STATUE_EYE);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x28);
    gb_write(&gb, wRoomObjects + 0x22, OBJECT_ONE_EYED_STATUE);
    gb_write(&gb, wRoomEventEffectExecuted, 0);

    ArrowEntityHandler(&gb, bc);
    assert(gb_read(&gb, wAttackDamageType) == DAMAGE_TYPE_ARROW);
    assert(gb_read(&gb, wRoomEventEffectExecuted) == 1);
    /* Entity should be unloaded */
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* Test 5: Wrong trigger or object fails statue eye */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityState, 0x00);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    gb_write_hram(&gb, hActiveEntitySpriteVariant, DIRECTION_UP);
    gb_write(&gb, wRoomEvent, 0x05); /* Wrong trigger */
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x28);
    gb_write(&gb, wRoomObjects + 0x22, OBJECT_ONE_EYED_STATUE);
    gb_write(&gb, wRoomEventEffectExecuted, 0);

    ArrowEntityHandler(&gb, bc);
    assert(gb_read(&gb, wRoomEventEffectExecuted) == 0);

    gb_write(&gb, wRoomEvent, TRIGGER_SHOOT_STATUE_EYE);
    gb_write(&gb, wRoomObjects + 0x22, 0x50); /* Wrong object */
    ArrowEntityHandler(&gb, bc);
    assert(gb_read(&gb, wRoomEventEffectExecuted) == 0);

    printf("[PASS] ArrowEntityHandler\n");
}

/* ===== 3. BombArrowHandler Verification ===== */
static void test_BombArrowHandler(void) {
    printf("[RUN ] BombArrowHandler\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x02;

    /* Setup source bomb arrow */
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x48);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x58);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x10);

    /* Test 1: Countdown != 0 spawns ENTITY_BOMB and unloads arrow */
    BombArrowHandler(&gb, bc);

    /* Slot bc should be unloaded */
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* New bomb should be spawned in available slot (highest slot: 15) */
    uint16_t bomb_slot = 15;
    assert(gb_read(&gb, wEntitiesStatusTable + bomb_slot) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wEntitiesTypeTable + bomb_slot) == ENTITY_BOMB);
    assert(gb_read(&gb, wEntitiesPosXTable + bomb_slot) == 0x48);
    assert(gb_read(&gb, wEntitiesPosYTable + bomb_slot) == 0x58);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bomb_slot) == 0x17);

    /* Test 2: Countdown == 0 (beforeExploding) sets damage type to BOMB_ARROW */
    gb_init(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x50);
    gb_write_hram(&gb, hActiveEntityPosX, 0x40);
    gb_write_hram(&gb, hActiveEntityPosY, 0x50);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x50);
    gb_write_hram(&gb, hActiveEntitySpriteVariant, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    gb_write(&gb, wAttackDamageType, 0x00);

    BombArrowHandler(&gb, bc);
    assert(gb_read(&gb, wAttackDamageType) == DAMAGE_TYPE_BOMB_ARROW);
    /* Active sprite variant restored */
    assert(gb_read_hram(&gb, hActiveEntitySpriteVariant) == DIRECTION_RIGHT);

    printf("[PASS] BombArrowHandler\n");
}

/* ===== 4. MoblinArrowEntityHandler Verification ===== */
static void test_MoblinArrowEntityHandler(void) {
    printf("[RUN ] MoblinArrowEntityHandler\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x01;

    /* Setup Moblin Arrow */
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_MOBLIN_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN_ARROW);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x30);
    gb_write_hram(&gb, hActiveEntityPosX, 0x30);
    gb_write_hram(&gb, hActiveEntityPosY, 0x30);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x30);

    /* Link is at (0x30, 0x30), unshielded */
    gb_write_hram(&gb, hLinkPositionX, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x30);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_DEFAULT);
    gb_write(&gb, wIsUsingShield, 0);

    /* Test 1: Countdown != 0 skips collision check */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x05);
    gb_write(&gb, wSubtractHealthBuffer, 0);
    MoblinArrowEntityHandler(&gb, bc);
    assert(gb_read(&gb, wSubtractHealthBuffer) == 0);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_ACTIVE);

    /* Test 2: Countdown == 0 checks collision with Link -> damages Link and unloads */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    MoblinArrowEntityHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    printf("[PASS] MoblinArrowEntityHandler\n");
}

/* ===== 5. ArrowRenderAndMove and skipRendering Verification ===== */
static void test_ArrowRenderAndMove(void) {
    printf("[RUN ] ArrowRenderAndMove\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x02;

    /* Test 1: Non-interactive entity returns early */
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_DISABLED);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    ArrowRenderAndMove(&gb, bc);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x20);

    /* Test 2: Countdown != 0 delegates to ArrowRockAfterHittingWall */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x01);
    ArrowRenderAndMove(&gb, bc);
    /* Countdown 1 unloads entity */
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* Test 3: Countdown == 0, no collision -> updates position with speed */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x20);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x40);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x00);
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x00);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);

    ArrowRenderAndMove(&gb, bc);
    assert(gb_read(&gb, wEntitiesPosXTable + bc) == 0x24);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x00);

    /* Test 4: Collision with Magic Rod Fireball */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_MAGIC_ROD_FIREBALL);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MAGIC_ROD_FIREBALL);
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x01);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);

    ArrowRenderAndMove(&gb, bc);
    assert(gb_read(&gb, wEntitiesPrivateCountdown1Table + bc) == 0x30);

    /* Test 5: Collision with Player Arrow (bounces with >> 2) */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x01);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x20); /* +32 -> -32 >> 2 = -8 = 0xF8 */
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0xE0); /* -32 -> +32 >> 2 = +8 = 0x08 */

    ArrowRenderAndMove(&gb, bc);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x18);
    assert(gb_read(&gb, wEntitiesSpeedZTable + bc) == 0x10);
    assert(gb_read_hram(&gb, hJingle) == JINGLE_SWORD_POKING);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0xF8);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x08);

    /* Test 6: Collision with Moblin Arrow (bounces with >> 3, skip poking sound if collisions == 0xFF) */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_MOBLIN_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN_ARROW);
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0xFF); /* Shield blocked -> skips poking sound */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x20); /* +32 -> -32 >> 3 = -4 = 0xFC */
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0xE0); /* -32 -> +32 >> 3 = +4 = 0x04 */
    gb_write_hram(&gb, hJingle, 0x00);

    ArrowRenderAndMove(&gb, bc);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x18);
    assert(gb_read(&gb, wEntitiesSpeedZTable + bc) == 0x10);
    assert(gb_read_hram(&gb, hJingle) == 0x00); /* Sound skipped */
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0xFC);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x04);

    printf("[PASS] ArrowRenderAndMove\n");
}

/* ===== 6. EntityBounceOffWallX and EntityBounceOffWallY Verification ===== */
static void test_EntityBounceOffWallX_and_Y(void) {
    printf("[RUN ] EntityBounceOffWallX_and_Y\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x04;

    /* Test X bounce: positive speed (+16 = 0x10 -> -16 >> 3 = -2 = 0xFE) */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    EntityBounceOffWallX(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0xFE);

    /* Test X bounce: negative speed (-16 = 0xF0 -> +16 >> 3 = +2 = 0x02) */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0xF0);
    EntityBounceOffWallX(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x02);

    /* Test Y bounce: positive speed (+32 = 0x20 -> -32 >> 3 = -4 = 0xFC) */
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x20);
    EntityBounceOffWallY(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0xFC);

    /* Test Y bounce: negative speed (-32 = 0xE0 -> +32 >> 3 = +4 = 0x04) */
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0xE0);
    EntityBounceOffWallY(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x04);

    /* Test zero speed stays zero */
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x00);
    EntityBounceOffWallX(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x00);

    /* Test NULL gb safety */
    EntityBounceOffWallX(NULL, bc);
    EntityBounceOffWallY(NULL, bc);

    printf("[PASS] EntityBounceOffWallX_and_Y\n");
}

/* ===== 7. ArrowRockAfterHittingWall Verification ===== */
static void test_ArrowRockAfterHittingWall(void) {
    printf("[RUN ] ArrowRockAfterHittingWall\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x01;

    /* Test 1: Countdown == 1 unloads entity */
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x01);
    ArrowRockAfterHittingWall(&gb, bc);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* Test 2: Arrow spinning animation based on countdown */
    gb_init(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_ARROW);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_ARROW);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x20);
    gb_write(&gb, wEntitiesPosZTable + bc, 0x10);

    /* Countdown 0x08 -> frame (8 >> 3) & 3 = 1 -> DIRECTION_DOWN (0) */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x08);
    ArrowRockAfterHittingWall(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == DIRECTION_DOWN);

    /* Countdown 0x10 -> frame (16 >> 3) & 3 = 2 -> DIRECTION_LEFT (1) */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x10);
    ArrowRockAfterHittingWall(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == DIRECTION_LEFT);

    /* Countdown 0x18 -> frame (24 >> 3) & 3 = 3 -> DIRECTION_UP (2) */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x18);
    ArrowRockAfterHittingWall(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == DIRECTION_UP);

    /* Countdown 0x20 -> frame (32 >> 3) & 3 = 0 -> DIRECTION_RIGHT (6 or 0) */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x20);
    ArrowRockAfterHittingWall(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == DIRECTION_RIGHT);

    /* Test 3: Octorok Rock does not spin */
    gb_init(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_OCTOROK_ROCK);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK_ROCK);
    gb_write(&gb, wEntitiesSpriteVariantTable + bc, 0x55);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x18);

    ArrowRockAfterHittingWall(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == 0x55);

    /* Test NULL gb safety */
    ArrowRockAfterHittingWall(NULL, bc);

    printf("[PASS] ArrowRockAfterHittingWall\n");
}

/* ===== Main Test Entry Point ===== */
void test_bank3_entities_arrow(void) {
    printf("--- Running Bank 3 Arrow Subsystem Tests ---\n");

    test_DataTables_Arrow();
    test_ArrowEntityHandler();
    test_BombArrowHandler();
    test_MoblinArrowEntityHandler();
    test_ArrowRenderAndMove();
    test_EntityBounceOffWallX_and_Y();
    test_ArrowRockAfterHittingWall();

    printf("--- Bank 3 Arrow Subsystem Tests Passed! ---\n\n");
}
