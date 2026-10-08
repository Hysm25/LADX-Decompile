#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "gb.h"
#include "bank3/entities_moblin.h"
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
#include "constants/rooms.h"

/* Helper to setup interactive game state so ReturnIfNonInteractive_03 passes */
static void setup_interactive(GBState *gb) {
    gb_write_hram(gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(gb, wTransitionSequenceCounter, 0x04);
    gb_write(gb, wDialogState, 0x00);
    gb_write(gb, wC1A8, 0x00);
    gb_write(gb, wInventoryAppearing, 0x00);
    gb_write(gb, wRoomTransitionState, 0x00);
}

/* ===== 1. Data Tables Verification ===== */
static void test_DataTables_Moblin(void) {
    printf("[RUN ] DataTables_Moblin\n");

    /* OctorokSpriteVariants (32 bytes: 8 variants * 4 bytes) */
    assert(sizeof(OctorokSpriteVariants) == 32);
    /* variant 0 (down 0) */
    assert(OctorokSpriteVariants[0] == 0x30);
    assert(OctorokSpriteVariants[1] == (OAM_GBC_PAL_2 | OAMF_PAL0));
    assert(OctorokSpriteVariants[2] == 0x30);
    assert(OctorokSpriteVariants[3] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP));
    /* variant 1 (down 1) */
    assert(OctorokSpriteVariants[4] == 0x32);
    assert(OctorokSpriteVariants[5] == (OAM_GBC_PAL_2 | OAMF_PAL0));
    assert(OctorokSpriteVariants[6] == 0x32);
    assert(OctorokSpriteVariants[7] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP));
    /* variant 2 (up 0) */
    assert(OctorokSpriteVariants[8] == 0x30);
    assert(OctorokSpriteVariants[9] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_YFLIP));
    assert(OctorokSpriteVariants[10] == 0x30);
    assert(OctorokSpriteVariants[11] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_YFLIP | OAMF_XFLIP));
    /* variant 3 (up 1) */
    assert(OctorokSpriteVariants[12] == 0x32);
    assert(OctorokSpriteVariants[13] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_YFLIP));
    assert(OctorokSpriteVariants[14] == 0x32);
    assert(OctorokSpriteVariants[15] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_YFLIP | OAMF_XFLIP));
    /* variant 4 (left 0) */
    assert(OctorokSpriteVariants[16] == 0x34);
    assert(OctorokSpriteVariants[17] == (OAM_GBC_PAL_2 | OAMF_PAL0));
    assert(OctorokSpriteVariants[18] == 0x36);
    assert(OctorokSpriteVariants[19] == (OAM_GBC_PAL_2 | OAMF_PAL0));
    /* variant 5 (left 1) */
    assert(OctorokSpriteVariants[20] == 0x38);
    assert(OctorokSpriteVariants[21] == (OAM_GBC_PAL_2 | OAMF_PAL0));
    assert(OctorokSpriteVariants[22] == 0x3A);
    assert(OctorokSpriteVariants[23] == (OAM_GBC_PAL_2 | OAMF_PAL0));
    /* variant 6 (right 0) */
    assert(OctorokSpriteVariants[24] == 0x36);
    assert(OctorokSpriteVariants[25] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP));
    assert(OctorokSpriteVariants[26] == 0x34);
    assert(OctorokSpriteVariants[27] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP));
    /* variant 7 (right 1) */
    assert(OctorokSpriteVariants[28] == 0x3A);
    assert(OctorokSpriteVariants[29] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP));
    assert(OctorokSpriteVariants[30] == 0x38);
    assert(OctorokSpriteVariants[31] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP));

    /* RoamingEnemySpeedXPerDirection & Y */
    assert(sizeof(RoamingEnemySpeedXPerDirection) == 4);
    assert(RoamingEnemySpeedXPerDirection[0] == 8);
    assert(RoamingEnemySpeedXPerDirection[1] == -8);
    assert(RoamingEnemySpeedXPerDirection[2] == 0);
    assert(RoamingEnemySpeedXPerDirection[3] == 0);

    assert(sizeof(RoamingEnemySpeedYPerDirection) == 4);
    assert(RoamingEnemySpeedYPerDirection[0] == 0);
    assert(RoamingEnemySpeedYPerDirection[1] == 0);
    assert(RoamingEnemySpeedYPerDirection[2] == -8);
    assert(RoamingEnemySpeedYPerDirection[3] == 8);

    /* EntityVariantForDirection_03 */
    assert(sizeof(EntityVariantForDirection_03) == 4);
    assert(EntityVariantForDirection_03[0] == 6); /* right */
    assert(EntityVariantForDirection_03[1] == 4); /* left */
    assert(EntityVariantForDirection_03[2] == 2); /* up */
    assert(EntityVariantForDirection_03[3] == 0); /* down */

    /* MoblinSpriteVariants (32 bytes) */
    assert(sizeof(MoblinSpriteVariants) == 32);
    assert(MoblinSpriteVariants[0] == 0x60);
    assert(MoblinSpriteVariants[1] == OAM_GBC_PAL_3);
    assert(MoblinSpriteVariants[2] == 0x62);
    assert(MoblinSpriteVariants[3] == OAM_GBC_PAL_3);
    assert(MoblinSpriteVariants[4] == 0x62);
    assert(MoblinSpriteVariants[5] == (OAM_GBC_PAL_3 | OAMF_XFLIP));
    assert(MoblinSpriteVariants[6] == 0x60);
    assert(MoblinSpriteVariants[7] == (OAM_GBC_PAL_3 | OAMF_XFLIP));

    /* MoblinArrow offsets & speeds */
    assert(sizeof(MoblinArrowOffsetXPerDirection) == 4);
    assert(MoblinArrowOffsetXPerDirection[0] == 8);
    assert(MoblinArrowOffsetXPerDirection[1] == -8);
    assert(MoblinArrowOffsetXPerDirection[2] == 4);
    assert(MoblinArrowOffsetXPerDirection[3] == -4);

    assert(sizeof(MoblinArrowOffsetYPerDirection) == 4);
    assert(MoblinArrowOffsetYPerDirection[0] == -4);
    assert(MoblinArrowOffsetYPerDirection[1] == -4);
    assert(MoblinArrowOffsetYPerDirection[2] == -8);
    assert(MoblinArrowOffsetYPerDirection[3] == 0);

    assert(sizeof(MoblinArrowSpeedXPerDirection) == 4);
    assert(MoblinArrowSpeedXPerDirection[0] == 32);
    assert(MoblinArrowSpeedXPerDirection[1] == -32);
    assert(MoblinArrowSpeedXPerDirection[2] == 0);
    assert(MoblinArrowSpeedXPerDirection[3] == 0);

    assert(sizeof(MoblinArrowSpeedYPerDirection) == 4);
    assert(MoblinArrowSpeedYPerDirection[0] == 0);
    assert(MoblinArrowSpeedYPerDirection[1] == 0);
    assert(MoblinArrowSpeedYPerDirection[2] == -32);
    assert(MoblinArrowSpeedYPerDirection[3] == 32);

    /* OctorokRock offsets & speeds */
    assert(sizeof(OctorokRockOffsetXPerDirection) == 4);
    assert(OctorokRockOffsetXPerDirection[0] == 8);
    assert(OctorokRockOffsetXPerDirection[1] == -8);
    assert(OctorokRockOffsetXPerDirection[2] == 0);
    assert(OctorokRockOffsetXPerDirection[3] == 0);

    assert(sizeof(OctorokRockOffsetYPerDirection) == 4);
    assert(OctorokRockOffsetYPerDirection[0] == 0);
    assert(OctorokRockOffsetYPerDirection[1] == 0);
    assert(OctorokRockOffsetYPerDirection[2] == -8);
    assert(OctorokRockOffsetYPerDirection[3] == 8);

    assert(sizeof(OctorokRockSpeedXPerDirection) == 4);
    assert(OctorokRockSpeedXPerDirection[0] == 32);
    assert(OctorokRockSpeedXPerDirection[1] == -32);
    assert(OctorokRockSpeedXPerDirection[2] == 0);
    assert(OctorokRockSpeedXPerDirection[3] == 0);

    assert(sizeof(OctorokRockSpeedYPerDirection) == 4);
    assert(OctorokRockSpeedYPerDirection[0] == 0);
    assert(OctorokRockSpeedYPerDirection[1] == 0);
    assert(OctorokRockSpeedYPerDirection[2] == -32);
    assert(OctorokRockSpeedYPerDirection[3] == 32);

    /* MaskedIronMaskSpriteVariants */
    assert(sizeof(MaskedIronMaskSpriteVariants) == 32);
    assert(MaskedIronMaskSpriteVariants[0] == 0x58);
    assert(MaskedIronMaskSpriteVariants[1] == OAM_GBC_PAL_3);

    printf("[PASS] DataTables_Moblin\n");
}

/* ===== 2. OctorokEntityHandler Verification ===== */
static void test_OctorokEntityHandler(void) {
    printf("[RUN ] OctorokEntityHandler\n");

    GBState gb;
    gb_init(&gb);
    setup_interactive(&gb);
    uint16_t bc = 0x01;

    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_OCTOROK);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write_hram(&gb, hActiveEntityState, 0x00);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);

    OctorokEntityHandler(&gb, bc);
    /* In non-credits gameplay, sets tiles offset to 0x30 */
    assert(gb_read_hram(&gb, hActiveEntityTilesOffset) == 0x30);

    /* Test credits gameplay leaves tiles offset untouched */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_OCTOROK);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write_hram(&gb, hActiveEntityState, 0x00);
    gb_write(&gb, wGameplayType, GAMEPLAY_CREDITS);
    gb_write_hram(&gb, hActiveEntityTilesOffset, 0x12);

    OctorokEntityHandler(&gb, bc);
    assert(gb_read_hram(&gb, hActiveEntityTilesOffset) == 0x12);

    /* NULL gb check */
    OctorokEntityHandler(NULL, bc);

    printf("[PASS] OctorokEntityHandler\n");
}

/* ===== 3. MoblinEntityHandler Verification ===== */
static void test_MoblinEntityHandler(void) {
    printf("[RUN ] MoblinEntityHandler\n");

    GBState gb;
    gb_init(&gb);
    setup_interactive(&gb);
    uint16_t bc = 0x03;

    /* Test 1: In BowWow hideout and BowWow NOT following Link -> unloads entity */
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write_hram(&gb, hMapId, MAP_BOWWOW_HIDEOUT);
    gb_write(&gb, wIsBowWowFollowingLink, 0x00);

    MoblinEntityHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_DISABLED);

    /* Test 2: In BowWow hideout and BowWow IS following Link ($80) -> preserved, records index in wD153 */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_MOBLIN);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN);
    gb_write_hram(&gb, hMapId, MAP_BOWWOW_HIDEOUT);
    gb_write(&gb, wIsBowWowFollowingLink, 0x80);

    MoblinEntityHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wD153) == 0x03);

    /* Test 3: Normal room outside hideout -> preserved, wD153 updated */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_MOBLIN);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN);
    gb_write_hram(&gb, hMapId, 0x20); /* Other room */
    gb_write(&gb, wIsBowWowFollowingLink, 0x00);

    MoblinEntityHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStatusTable + bc) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wD153) == 0x03);

    /* NULL gb check */
    MoblinEntityHandler(NULL, bc);

    printf("[PASS] MoblinEntityHandler\n");
}

/* ===== 4. AnimateRoamingEnemy Verification ===== */
static void test_AnimateRoamingEnemy(void) {
    printf("[RUN ] AnimateRoamingEnemy\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x02;

    /* Test 1: Non-interactive returns early */
    gb_write_hram(&gb, hActiveEntityStatus, ENTITY_STATUS_DISABLED);
    AnimateRoamingEnemy(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0);

    /* Test 2: Recoil triggered when IgnoreHitsCountdown != 0 */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_MOBLIN);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN);
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + bc, 0x05);
    gb_write_hram(&gb, hActiveEntityState, 0x00);

    AnimateRoamingEnemy(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x01);
    assert(gb_read_hram(&gb, hActiveEntityState) == 0x01);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x40);

    /* Test 3: State != 0 and TransitionCountdown == 0 -> picks new direction and speed */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_MOBLIN);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN);
    gb_write_hram(&gb, hActiveEntityState, 0x01);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    gb_write(&gb, wEntitiesPrivateState1Table + bc, 0x03); /* +1 becomes 0 -> towards Link */
    gb_write(&gb, wRandomSeed, 0x15);
    /* Link to the right of entity */
    gb_write_hram(&gb, hLinkPositionX, 0x60);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x20);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);

    AnimateRoamingEnemy(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesPrivateState1Table + bc) == 0x00);
    /* Countdown set to (rand & 0x1F) | 0x20 */
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) >= 0x20);
    /* Direction towards Link (right = 0) */
    assert(gb_read(&gb, wEntitiesDirectionTable + bc) == DIRECTION_RIGHT);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 8);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0);

    /* Test 4: Projectile shooting at Countdown == 0x0A facing Link */
    /* Moblin arrow shoot */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_MOBLIN);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_MOBLIN);
    gb_write_hram(&gb, hActiveEntityState, 0x01);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x0A);
    gb_write(&gb, wEntitiesPrivateCountdown1Table + bc, 0x00);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT);
    gb_write_hram(&gb, hLinkPositionX, 0x70);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);

    AnimateRoamingEnemy(&gb, bc);
    /* Moblin arrow should be spawned in available slot (e.g. slot 15) */
    bool found_arrow = false;
    for (uint16_t s = 0; s < MAX_ENTITIES; s++) {
        if (s != bc && gb_read(&gb, wEntitiesStatusTable + s) == ENTITY_STATUS_ACTIVE &&
            gb_read(&gb, wEntitiesTypeTable + s) == ENTITY_MOBLIN_ARROW) {
            found_arrow = true;
            break;
        }
    }
    assert(found_arrow);

    /* Octorok rock shoot */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_OCTOROK);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_OCTOROK);
    gb_write_hram(&gb, hActiveEntityState, 0x01);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x0A);
    gb_write(&gb, wEntitiesPrivateCountdown1Table + bc, 0x00);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT);
    gb_write_hram(&gb, hLinkPositionX, 0x70);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);

    AnimateRoamingEnemy(&gb, bc);
    bool found_rock = false;
    for (uint16_t s = 0; s < MAX_ENTITIES; s++) {
        if (s != bc && gb_read(&gb, wEntitiesStatusTable + s) == ENTITY_STATUS_ACTIVE &&
            gb_read(&gb, wEntitiesTypeTable + s) == ENTITY_OCTOROK_ROCK) {
            found_rock = true;
            break;
        }
    }
    assert(found_rock);

    /* Iron mask does NOT shoot projectile */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesTypeTable + bc, ENTITY_IRON_MASK);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_IRON_MASK);
    gb_write_hram(&gb, hActiveEntityState, 0x01);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x0A);
    gb_write(&gb, wEntitiesPrivateCountdown1Table + bc, 0x00);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT);
    gb_write_hram(&gb, hLinkPositionX, 0x70);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write_hram(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);

    AnimateRoamingEnemy(&gb, bc);
    for (uint16_t s = 0; s < MAX_ENTITIES; s++) {
        if (s != bc) {
            assert(gb_read(&gb, wEntitiesStatusTable + s) == ENTITY_STATUS_DISABLED);
        }
    }

    printf("[PASS] AnimateRoamingEnemy\n");
}

/* ===== 5. RoamingEnemyState0Handler Verification ===== */
static void test_RoamingEnemyState0Handler(void) {
    printf("[RUN ] RoamingEnemyState0Handler\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x01;

    /* Test 1: Wall collision detected (collisions & 0x0F != 0) -> state=1, clears speed, transition countdown untouched, collisions reset by background */
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x01);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x25);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x08);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x08);
    gb_write(&gb, wEntitiesStateTable + bc, 0x00);

    RoamingEnemyState0Handler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x01);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesTransitionCountdownTable + bc) == 0x25);
    assert(gb_read(&gb, wEntitiesCollisionsTable + bc) == 0x00);

    /* Test 2: No collision, Countdown != 0 -> keeps walking */
    gb_init(&gb);
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x00);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x15);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x10);
    gb_write(&gb, wEntitiesStateTable + bc, 0x00);

    RoamingEnemyState0Handler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x00);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x10);

    /* Test 3: No collision, Countdown == 0 -> writes random to transition countdown, state=1, clears speed */
    gb_init(&gb);
    gb_write(&gb, wEntitiesCollisionsTable + bc, 0x00);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x08);
    gb_write(&gb, wEntitiesStateTable + bc, 0x00);

    RoamingEnemyState0Handler(&gb, bc);
    assert(gb_read(&gb, wEntitiesStateTable + bc) == 0x01);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x00);
    assert((gb_read(&gb, wEntitiesTransitionCountdownTable + bc) & 0x10) != 0);

    /* NULL check */
    RoamingEnemyState0Handler(NULL, bc);

    printf("[PASS] RoamingEnemyState0Handler\n");
}

/* ===== 6. SetEntityVariantForDirection_03 Verification ===== */
static void test_SetEntityVariantForDirection_03(void) {
    printf("[RUN ] SetEntityVariantForDirection_03\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x02;

    /* Test direction 0 (RIGHT): base variant is 6 */
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_RIGHT);
    gb_write(&gb, wEntitiesInertiaTable + bc, 0x00); /* Will become 1 -> bit 3 is 0 -> variant 6 */
    SetEntityVariantForDirection_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == 6);

    /* After 7 more calls (inertia reaches 8 -> bit 3 is 1) -> variant 7 */
    for (int i = 0; i < 7; i++) {
        SetEntityVariantForDirection_03(&gb, bc);
    }
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == 7);

    /* Test direction 1 (LEFT): base variant 4 */
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_LEFT);
    gb_write(&gb, wEntitiesInertiaTable + bc, 0x00);
    SetEntityVariantForDirection_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == 4);

    /* Test direction 2 (UP): base variant 2 */
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_UP);
    gb_write(&gb, wEntitiesInertiaTable + bc, 0x00);
    SetEntityVariantForDirection_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == 2);

    /* Test direction 3 (DOWN): base variant 0 */
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_DOWN);
    gb_write(&gb, wEntitiesInertiaTable + bc, 0x00);
    SetEntityVariantForDirection_03(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == 0);

    /* NULL check */
    SetEntityVariantForDirection_03(NULL, bc);

    printf("[PASS] SetEntityVariantForDirection_03\n");
}

/* ===== 7. SpawnMoblinArrow Verification ===== */
static void test_SpawnMoblinArrow(void) {
    printf("[RUN ] SpawnMoblinArrow\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x01;

    /* Setup parent Moblin */
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x40);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x50);
    gb_write_hram(&gb, hMultiPurpose0, 0x40);
    gb_write_hram(&gb, hMultiPurpose1, 0x50);
    gb_write_hram(&gb, hMultiPurpose2, DIRECTION_RIGHT);

    SpawnMoblinArrow(&gb, bc);

    /* Find spawned arrow (should be slot 15 = MAX_ENTITIES - 1) */
    uint16_t arrow_slot = MAX_ENTITIES - 1;
    assert(gb_read(&gb, wEntitiesStatusTable + arrow_slot) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wEntitiesTypeTable + arrow_slot) == ENTITY_MOBLIN_ARROW);
    /* Direction RIGHT: offset X = +8, offset Y = -4 */
    assert(gb_read(&gb, wEntitiesPosXTable + arrow_slot) == 0x48);
    assert(gb_read(&gb, wEntitiesPosYTable + arrow_slot) == 0x4C);
    /* Speed RIGHT: speed X = 32 (0x20), speed Y = 0 */
    assert(gb_read(&gb, wEntitiesSpeedXTable + arrow_slot) == 0x20);
    assert(gb_read(&gb, wEntitiesSpeedYTable + arrow_slot) == 0x00);
    assert(gb_read(&gb, wEntitiesDirectionTable + arrow_slot) == DIRECTION_RIGHT);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + arrow_slot) == DIRECTION_RIGHT);

    /* Test slot exhaustion returns safely */
    for (uint16_t i = 0; i < MAX_ENTITIES; i++) {
        gb_write(&gb, wEntitiesStatusTable + i, ENTITY_STATUS_ACTIVE);
    }
    SpawnMoblinArrow(&gb, bc);

    /* NULL check */
    SpawnMoblinArrow(NULL, bc);

    printf("[PASS] SpawnMoblinArrow\n");
}

/* ===== 8. SpawnOctorokRock Verification ===== */
static void test_SpawnOctorokRock(void) {
    printf("[RUN ] SpawnOctorokRock\n");

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x01;

    /* Setup parent Octorok */
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPosXTable + bc, 0x30);
    gb_write(&gb, wEntitiesPosYTable + bc, 0x40);
    gb_write(&gb, wEntitiesDirectionTable + bc, DIRECTION_UP);
    gb_write_hram(&gb, hMultiPurpose0, 0x30);
    gb_write_hram(&gb, hMultiPurpose1, 0x40);
    gb_write_hram(&gb, hMultiPurpose2, DIRECTION_UP);

    SpawnOctorokRock(&gb, bc);

    /* Find spawned rock (slot 15) */
    uint16_t rock_slot = MAX_ENTITIES - 1;
    assert(gb_read(&gb, wEntitiesStatusTable + rock_slot) == ENTITY_STATUS_ACTIVE);
    assert(gb_read(&gb, wEntitiesTypeTable + rock_slot) == ENTITY_OCTOROK_ROCK);
    /* Direction UP: offset X = 0, offset Y = -8 */
    assert(gb_read(&gb, wEntitiesPosXTable + rock_slot) == 0x30);
    assert(gb_read(&gb, wEntitiesPosYTable + rock_slot) == 0x38);
    /* Speed UP: speed X = 0, speed Y = -32 (0xE0) */
    assert(gb_read(&gb, wEntitiesSpeedXTable + rock_slot) == 0x00);
    assert(gb_read(&gb, wEntitiesSpeedYTable + rock_slot) == 0xE0);
    assert(gb_read(&gb, wEntitiesDirectionTable + rock_slot) == DIRECTION_UP);

    /* Test slot exhaustion returns safely */
    for (uint16_t i = 0; i < MAX_ENTITIES; i++) {
        gb_write(&gb, wEntitiesStatusTable + i, ENTITY_STATUS_ACTIVE);
    }
    SpawnOctorokRock(&gb, bc);

    /* NULL check */
    SpawnOctorokRock(NULL, bc);

    printf("[PASS] SpawnOctorokRock\n");
}

/* ===== IronMaskEntityHandler Test ===== */
static void test_IronMaskEntityHandler(void) {
    printf("[RUN ] IronMaskEntityHandler\n");

    /* 1. Verify Data Tables */
    assert(sizeof(UnmaskedIronMaskSpriteVariants) == 8);
    assert(UnmaskedIronMaskSpriteVariants[0] == 0x70);
    assert(UnmaskedIronMaskSpriteVariants[1] == (OAM_GBC_PAL_2 | OAMF_PAL0));
    assert(UnmaskedIronMaskSpriteVariants[2] == 0x72);
    assert(UnmaskedIronMaskSpriteVariants[3] == (OAM_GBC_PAL_2 | OAMF_PAL0));
    assert(UnmaskedIronMaskSpriteVariants[4] == 0x72);
    assert(UnmaskedIronMaskSpriteVariants[5] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP));
    assert(UnmaskedIronMaskSpriteVariants[6] == 0x70);
    assert(UnmaskedIronMaskSpriteVariants[7] == (OAM_GBC_PAL_2 | OAMF_PAL0 | OAMF_XFLIP));

    assert(IronMaskSpeedXValues[0] == 0x0C);
    assert(IronMaskSpeedXValues[1] == (int8_t)0xF4);
    assert(IronMaskSpeedXValues[2] == 0x00);
    assert(IronMaskSpeedXValues[3] == 0x00);

    assert(IronMaskSpeedYValues[0] == 0x00);
    assert(IronMaskSpeedYValues[1] == 0x00);
    assert(IronMaskSpeedYValues[2] == (int8_t)0xF4);
    assert(IronMaskSpeedYValues[3] == 0x0C);

    GBState gb;
    gb_init(&gb);
    uint16_t bc = 0x01;

    /* 2. Masked path (unmasked == 0): dispatches to AnimateRoamingEnemy */
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesPrivateState2Table + bc, 0x00);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wActiveEntityIndex, (uint8_t)bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_IRON_MASK);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x20);
    gb_write_hram(&gb, hActiveEntityPosY, 0x20);
    gb_write_hram(&gb, hActiveEntityPosX, 0x20);
    IronMaskEntityHandler(&gb, bc);

    /* 3. Unmasked path (unmasked == 1) with countdown == 0 */
    gb_init(&gb);
    setup_interactive(&gb);
    gb_write(&gb, wEntitiesPrivateState2Table + bc, 0x01);
    gb_write(&gb, wEntitiesStatusTable + bc, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wActiveEntityIndex, (uint8_t)bc);
    gb_write_hram(&gb, hActiveEntityType, ENTITY_IRON_MASK);
    gb_write_hram(&gb, hActiveEntityVisualPosY, 0x20);
    gb_write_hram(&gb, hActiveEntityPosY, 0x20);
    gb_write_hram(&gb, hActiveEntityPosX, 0x20);
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x00);
    /* Frame counter for variant (frame >> 4) & 1 */
    gb_write_hram(&gb, hFrameCounter, 0x10); /* 0x10 >> 4 = 1 */

    IronMaskEntityHandler(&gb, bc);

    /* Verify transition countdown set to (rand & 0x1F) + 0x20 */
    uint8_t timer = gb_read(&gb, wEntitiesTransitionCountdownTable + bc);
    assert(timer >= 0x20 && timer <= 0x3F);
    uint8_t dir = (uint8_t)(timer & 0x03);
    (void)dir;
    assert((int8_t)gb_read(&gb, wEntitiesSpeedXTable + bc) == IronMaskSpeedXValues[dir]);
    assert((int8_t)gb_read(&gb, wEntitiesSpeedYTable + bc) == IronMaskSpeedYValues[dir]);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == 1);

    /* 4. Unmasked path with countdown != 0: retains speeds, updates frame variant */
    gb_write(&gb, wEntitiesTransitionCountdownTable + bc, 0x10);
    gb_write(&gb, wEntitiesSpeedXTable + bc, 0x05);
    gb_write(&gb, wEntitiesSpeedYTable + bc, 0x06);
    gb_write_hram(&gb, hFrameCounter, 0x00); /* 0x00 >> 4 = 0 */
    IronMaskEntityHandler(&gb, bc);
    assert(gb_read(&gb, wEntitiesSpeedXTable + bc) == 0x05);
    assert(gb_read(&gb, wEntitiesSpeedYTable + bc) == 0x06);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + bc) == 0);

    /* 5. NULL check */
    IronMaskEntityHandler(NULL, bc);

    printf("[PASS] IronMaskEntityHandler\n");
}

/* ===== Entry Point for Bank 3 Moblin/Roaming Enemy Tests ===== */
void test_bank3_entities_moblin(void) {
    test_DataTables_Moblin();
    test_OctorokEntityHandler();
    test_MoblinEntityHandler();
    test_AnimateRoamingEnemy();
    test_RoamingEnemyState0Handler();
    test_SetEntityVariantForDirection_03();
    test_SpawnMoblinArrow();
    test_SpawnOctorokRock();
    test_IronMaskEntityHandler();
}
