#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "gb.h"
#include "home/entities.h"
#include "constants/memory.h"
#include "constants/entities.h"
#include "constants/gameplay.h"
#include "constants/maps.h"
#include "constants/sfx.h"
#include "constants/audio.h"
#include "constants/dialog.h"

void test_is_zero(void) {

    GBState gb;
    gb_init(&gb);

    gb_write(&gb, 0xC200, 0x00);
    gb_write(&gb, 0xC205, 0x42);

    assert(IsZero(&gb, 0xC200, 0) == 0);
    assert(IsZero(&gb, 0xC200, 5) == 0x42);
}

void test_entity_countdowns(void) {

    GBState gb;
    gb_init(&gb);

    /* Test slow transition countdown */
    gb_write(&gb, wEntitiesSlowTransitionCountdownTable + 3, 0x05);
    assert(GetEntitySlowTransitionCountdown(&gb, 3) == 0x05);

    /* Test private countdown 1 */
    gb_write(&gb, wEntitiesPrivateCountdown1Table + 2, 0x09);
    assert(GetEntityPrivateCountdown1(&gb, 2) == 0x09);

    /* Test transition countdown */
    gb_write(&gb, wEntitiesTransitionCountdownTable + 1, 0x07);
    assert(GetEntityTransitionCountdown(&gb, 1) == 0x07);

    /* Test ignore hits countdown decrement */
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + 4, 0x03);
    DecrementEntityIgnoreHitsCountdown(&gb, 4);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + 4) == 0x02);

    DecrementEntityIgnoreHitsCountdown(&gb, 4);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + 4) == 0x01);

    DecrementEntityIgnoreHitsCountdown(&gb, 4);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + 4) == 0x00);

    /* Verify doesn't decrement below 0 */
    DecrementEntityIgnoreHitsCountdown(&gb, 4);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + 4) == 0x00);
}

static int spawn_called = 0;
static uint16_t mock_spawn_entity(GBState *gb, uint8_t entity_type) {
    spawn_called++;
    assert(gb->rom_bank == 0x03);
    assert(entity_type == ENTITY_TRADING_ITEM);
    return 0x05;
}

static uint16_t mock_spawn_in_range(GBState *gb, uint8_t entity_type) {
    spawn_called++;
    assert(gb->rom_bank == 0x03);
    assert(entity_type == 0x10);
    return 0x08;
}

static uint16_t mock_spawn_trampoline_wrapper(GBState *gb, uint8_t entity_type) {
    return SpawnNewEntity_trampoline(gb, entity_type, mock_spawn_entity);
}

void test_create_trading_item_entity(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x01);
    gb.rom_bank = 0x01;
    gb_write(&gb, hLinkPositionX, 0x48);
    gb_write(&gb, hLinkPositionY, 0x60);

    spawn_called = 0;
    CreateTradingItemEntity(&gb, mock_spawn_trampoline_wrapper);

    assert(spawn_called == 1);
    assert(gb.rom_bank == 0x01); /* Trampoline restored saved bank */
    assert(gb_read(&gb, wEntitiesPosXTable + 5) == 0x48);
    assert(gb_read(&gb, wEntitiesPosYTable + 5) == 0x60);
}

void test_spawn_entity_trampolines(void) {

    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x02);
    gb.rom_bank = 0x02;

    /* 1. SpawnNewEntity_trampoline */
    spawn_called = 0;
    uint16_t de = SpawnNewEntity_trampoline(&gb, ENTITY_TRADING_ITEM, mock_spawn_entity);
    assert(spawn_called == 1);
    assert(de == 0x05);
    assert(gb.rom_bank == 0x02); /* ReloadSavedBank restored wCurrentBank */

    /* 2. SpawnNewEntityInRange_trampoline */
    spawn_called = 0;
    gb_write(&gb, wCurrentBank, 0x04);
    gb.rom_bank = 0x04;
    de = SpawnNewEntityInRange_trampoline(&gb, 0x10, mock_spawn_in_range);
    assert(spawn_called == 1);
    assert(de == 0x08);
    assert(gb.rom_bank == 0x04); /* ReloadSavedBank restored wCurrentBank */
}

static int animate_called = 0;
static void mock_animate_entities(GBState *gb) {
    animate_called++;
    assert(gb->rom_bank == 0x03);
}

void test_animate_entities_trampolines(void) {

    GBState gb;
    gb_init(&gb);

    /* 1. AnimateEntitiesAndRestoreBank17 */
    animate_called = 0;
    AnimateEntitiesAndRestoreBank17(&gb, mock_animate_entities);
    assert(animate_called == 1);
    assert(gb.rom_bank == 0x17);
    assert(gb_read(&gb, wCurrentBank) == 0x17);

    /* 2. AnimateEntitiesAndRestoreBank01 */
    animate_called = 0;
    AnimateEntitiesAndRestoreBank01(&gb, mock_animate_entities);
    assert(animate_called == 1);
    assert(gb.rom_bank == 0x01);
    assert(gb_read(&gb, wCurrentBank) == 0x01);

    /* 3. AnimateEntitiesAndRestoreBank02 */
    animate_called = 0;
    AnimateEntitiesAndRestoreBank02(&gb, mock_animate_entities);
    assert(animate_called == 1);
    assert(gb.rom_bank == 0x02);
    assert(gb_read(&gb, wCurrentBank) == 0x02);
}


static uint8_t mock_entities_rom[0x4000 * 0x40];
#define ROM_OFFSET(bank, addr) (((size_t)(bank) * 0x4000) + ((size_t)(addr) - 0x4000))

static bool mock_cb_called = false;
static void mock_check_bank_cb(GBState *gb, uint8_t expected_bank) {
    mock_cb_called = true;
    assert(gb->rom_bank == expected_bank);
}

static void mock_cb_19(GBState *gb) {
    mock_check_bank_cb(gb, 0x19);
    assert(gb_read(gb, wCurrentBank) == 0x19);
}
static void mock_cb_03(GBState *gb) { mock_check_bank_cb(gb, 0x03); }
static void mock_cb_14(GBState *gb) { mock_check_bank_cb(gb, 0x14); }
static void mock_cb_15(GBState *gb) { mock_check_bank_cb(gb, 0x15); }
static void mock_cb_04(GBState *gb) { mock_check_bank_cb(gb, 0x04); }
static void mock_cb_05(GBState *gb) { mock_check_bank_cb(gb, 0x05); }
static void mock_cb_06(GBState *gb) { mock_check_bank_cb(gb, 0x06); }
static void mock_cb_36(GBState *gb) { mock_check_bank_cb(gb, 0x36); }
static void mock_cb_01(GBState *gb) {
    mock_check_bank_cb(gb, 0x01);
    assert(gb_read(gb, wCurrentBank) == 0x01);
}

static void test_entities_batch_trampolines(void) {

    GBState gb;

    /* 1. CanBowWowEatEntity */
    gb_init(&gb);
    memset(mock_entities_rom, 0, sizeof(mock_entities_rom));
    mock_entities_rom[ROM_OFFSET(0x14, BowWowEatableEntitiesTable + 0x09)] = 0x01;
    mock_entities_rom[ROM_OFFSET(0x14, BowWowEatableEntitiesTable + 0x0A)] = 0x00;
    mock_entities_rom[ROM_OFFSET(0x14, BowWowEatableEntitiesTable + 0x1B)] = 0x01;
    gb_attach_rom(&gb, mock_entities_rom, sizeof(mock_entities_rom));

    assert(CanBowWowEatEntity(&gb, 0x09) == 0x01);
    assert(gb.rom_bank == 0x05);

    assert(CanBowWowEatEntity(&gb, 0x0A) == 0x00);
    assert(gb.rom_bank == 0x05);

    assert(CanBowWowEatEntity(&gb, 0x1B) == 0x01);
    assert(gb.rom_bank == 0x05);

    /* 2. label_3935 */
    gb_init(&gb);
    mock_cb_called = false;
    label_3935(&gb, mock_cb_19);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x03);
    assert(gb_read(&gb, wCurrentBank) == 0x03);

    /* 3. LiftableRockStartSmashingAnimation_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x07);
    gb.rom_bank = 0x07;
    mock_cb_called = false;
    LiftableRockStartSmashingAnimation_trampoline(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x07);

    /* 4. label_394D */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x06);
    gb.rom_bank = 0x06;
    mock_cb_called = false;
    label_394D(&gb, mock_cb_14);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x06);

    /* 5. CreateFollowingNpcEntity_trampoline */
    gb_init(&gb);
    mock_cb_called = false;
    CreateFollowingNpcEntity_trampoline(&gb, mock_cb_01);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x02);
    assert(gb_read(&gb, wCurrentBank) == 0x02);

    /* 6. ConfigureNewEntity_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x08);
    gb.rom_bank = 0x08;
    mock_cb_called = false;
    ConfigureNewEntity_trampoline(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x08);

    /* 7. GetEntityDirectionToLink_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    GetEntityDirectionToLink_trampoline(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 8. label_397B */
    gb_init(&gb);
    mock_cb_called = false;
    label_397B(&gb, mock_cb_14);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x03);
}


static void mock_cb_02(GBState *gb) {
    mock_check_bank_cb(gb, 0x02);
}

static void test_entities_batch_hitbox_and_collision_trampolines(void) {

    GBState gb;

    /* 1. ConfigureEntityHitbox - unmapped ROM (fallback table) */
    gb_init(&gb);
    gb_write(&gb, wEntitiesHitboxFlagsTable + 2, 0x14 | 0x03); /* offset 0x14 */
    ConfigureEntityHitbox(&gb, 2);
    assert(gb_read(&gb, wEntitiesHitboxPositionTable + 8) == 0x08);
    assert(gb_read(&gb, wEntitiesHitboxPositionTable + 9) == 0x13);
    assert(gb_read(&gb, wEntitiesHitboxPositionTable + 10) == 0x08);
    assert(gb_read(&gb, wEntitiesHitboxPositionTable + 11) == 0x13);

    /* 2. ConfigureEntityHitbox - mapped ROM */
    gb_init(&gb);
    memset(mock_entities_rom, 0, sizeof(mock_entities_rom));
    mock_entities_rom[HitboxPositions + 0x28 + 0] = 0x0C;
    mock_entities_rom[HitboxPositions + 0x28 + 1] = 0x07;
    mock_entities_rom[HitboxPositions + 0x28 + 2] = 0xFC;
    mock_entities_rom[HitboxPositions + 0x28 + 3] = 0x04;
    gb_attach_rom(&gb, mock_entities_rom, sizeof(mock_entities_rom));

    gb_write(&gb, wEntitiesHitboxFlagsTable + 5, 0x28 | 0x80); /* offset 0x28, ignore hits bit set */
    ConfigureEntityHitbox(&gb, 5);
    assert(gb_read(&gb, wEntitiesHitboxPositionTable + 20) == 0x0C);
    assert(gb_read(&gb, wEntitiesHitboxPositionTable + 21) == 0x07);
    assert(gb_read(&gb, wEntitiesHitboxPositionTable + 22) == 0xFC);
    assert(gb_read(&gb, wEntitiesHitboxPositionTable + 23) == 0x04);

    /* 3. SetEntitySpriteVariant */
    gb_init(&gb);
    SetEntitySpriteVariant(&gb, 4, 0x33);
    assert(gb_read(&gb, wEntitiesSpriteVariantTable + 4) == 0x33);

    /* 4. IncrementEntityState */
    gb_init(&gb);
    gb_write(&gb, wEntitiesStateTable + 3, 0x05);
    IncrementEntityState(&gb, 3);
    assert(gb_read(&gb, wEntitiesStateTable + 3) == 0x06);

    /* 5. HurtBySpikes_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x08);
    gb.rom_bank = 0x08;
    mock_cb_called = false;
    HurtBySpikes_trampoline(&gb, mock_cb_02);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x08);

    /* 6. ApplyEntityInteractionWithBackground_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x0A);
    gb.rom_bank = 0x0A;
    mock_cb_called = false;
    ApplyEntityInteractionWithBackground_trampoline(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x0A);

    /* 7. label_3B2E */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x0B);
    gb.rom_bank = 0x0B;
    mock_cb_called = false;
    label_3B2E(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x0B);

    /* 8. DefaultEnemyDamageCollisionHandler_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x0C);
    gb.rom_bank = 0x0C;
    mock_cb_called = false;
    DefaultEnemyDamageCollisionHandler_trampoline(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x0C);

    /* 9. label_3B44 */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x0D);
    gb.rom_bank = 0x0D;
    mock_cb_called = false;
    label_3B44(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x0D);

    /* 10. CheckLinkCollisionWithProjectile_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x0E);
    gb.rom_bank = 0x0E;
    mock_cb_called = false;
    CheckLinkCollisionWithProjectile_trampoline(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x0E);

    /* 11. CheckLinkCollisionWithEnemy_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x0F);
    gb.rom_bank = 0x0F;
    mock_cb_called = false;
    CheckLinkCollisionWithEnemy_trampoline(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x0F);

    /* 12. label_3B65 */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x10);
    gb.rom_bank = 0x10;
    mock_cb_called = false;
    label_3B65(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x10);

    /* 13. label_3B70 */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x11);
    gb.rom_bank = 0x11;
    mock_cb_called = false;
    label_3B70(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x11);

    /* 14. label_3B7B */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x12);
    gb.rom_bank = 0x12;
    mock_cb_called = false;
    label_3B7B(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x12);

    /* 15. ApplyVectorTowardsLink_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x13);
    gb.rom_bank = 0x13;
    mock_cb_called = false;
    ApplyVectorTowardsLink_trampoline(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x13);

    /* 16. GetVectorTowardsLink_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x14);
    gb.rom_bank = 0x14;
    mock_cb_called = false;
    GetVectorTowardsLink_trampoline(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x14);
}


static int test_func4303_called = 0;
static int test_func6352_called = 0;
static int test_animate_entity_count = 0;
static uint8_t test_animated_slots[16];

static void mock_func_020_4303(GBState *gb) {
    test_func4303_called++;
    assert(gb->rom_bank == 0x20);
}

static void mock_func_020_6352(GBState *gb) {
    test_func6352_called++;
    assert(gb->rom_bank == 0x20);
    assert(gb_read(gb, wCurrentBank) == 0x20);
}

static void mock_animate_entity_slot(GBState *gb, uint16_t entity_index) {
    test_animated_slots[test_animate_entity_count++] = (uint8_t)entity_index;
    assert(gb_read(gb, wActiveEntityIndex) == entity_index);
}

static bool s_simple_cb_called = false;
static void simple_test_cb(GBState *gb) { (void)gb; s_simple_cb_called = true; }
static int test_status_handler_called = 0;
static void mock_status_active_handler(GBState *gb) {
    test_status_handler_called++;
    assert(gb->rom_bank == 0x03);
}

static uint8_t dispatched_bank = 0;
static uint16_t dispatched_addr = 0;
static void mock_handler_dispatch(GBState *gb, uint8_t bank, uint16_t addr) {
    dispatched_bank = bank;
    dispatched_addr = addr;
}

static void test_animate_entities_pipeline(void) {

    GBState gb;

    /* 1. ClearEntitySpeed */
    gb_init(&gb);
    gb_write(&gb, wEntitiesSpeedXTable + 3, 0x12);
    gb_write(&gb, wEntitiesSpeedYTable + 3, 0x34);
    ClearEntitySpeed(&gb, 3);
    assert(gb_read(&gb, wEntitiesSpeedXTable + 3) == 0);
    assert(gb_read(&gb, wEntitiesSpeedYTable + 3) == 0);

    /* 2. CopyEntityPositionToActivePosition */
    gb_init(&gb);
    gb_write(&gb, wEntitiesPosXTable + 4, 0x50);
    gb_write(&gb, wEntitiesPosYTable + 4, 0x60);
    gb_write(&gb, wEntitiesPosZTable + 4, 0x08);
    CopyEntityPositionToActivePosition(&gb, 4);
    assert(gb_read(&gb, hActiveEntityPosX) == 0x50);
    assert(gb_read(&gb, hActiveEntityPosY) == 0x60);
    assert(gb_read(&gb, hActiveEntityVisualPosY) == 0x58);

    /* 3. ResetEntity_trampoline */
    gb_init(&gb);
    mock_cb_called = false;
    ResetEntity_trampoline(&gb, mock_cb_15); /* reuses checking bank */
    assert(gb.rom_bank == 0x03);

    /* 4. AnimateEntity */
    gb_init(&gb);
    gb_write(&gb, wEntitiesTypeTable + 2, 0x09);
    gb_write(&gb, wEntitiesStateTable + 2, 0x01);
    gb_write(&gb, wEntitiesSpriteVariantTable + 2, 0x04);
    gb_write(&gb, wEntitiesStatusTable + 2, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, hActiveEntityStatus, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesPosXTable + 2, 0x20);
    gb_write(&gb, wEntitiesPosYTable + 2, 0x30);
    gb_write(&gb, wEntitiesPosZTable + 2, 0x00);

    AnimateEntityCallbacks ae_cbs = {
        .ExecuteActiveEntityHandler = mock_status_active_handler,
    };
    test_status_handler_called = 0;
    AnimateEntity(&gb, 2, &ae_cbs);

    assert(test_status_handler_called == 1);
    assert(gb_read(&gb, hActiveEntityType) == 0x09);
    assert(gb_read(&gb, hActiveEntityState) == 0x01);
    assert(gb_read(&gb, hActiveEntitySpriteVariant) == 0x04);
    assert(gb_read(&gb, hActiveEntityPosX) == 0x20);
    assert(gb_read(&gb, hActiveEntityPosY) == 0x30);
    assert(gb.rom_bank == 0x03);

    /* 5. ExecuteActiveEntityHandler */
    gb_init(&gb);
    memset(mock_entities_rom, 0, sizeof(mock_entities_rom));
    /* EntityHandlersTable is at 0x4000 in bank 0x20 */
    /* Type 0x05 entry at 0x4000 + 0x05 * 3 = 0x400F */
    mock_entities_rom[ROM_OFFSET(0x20, EntityHandlersTable + 5 * 3 + 0)] = 0x78; /* low */
    mock_entities_rom[ROM_OFFSET(0x20, EntityHandlersTable + 5 * 3 + 1)] = 0x56; /* high */
    mock_entities_rom[ROM_OFFSET(0x20, EntityHandlersTable + 5 * 3 + 2)] = 0x0B; /* bank */
    gb_attach_rom(&gb, mock_entities_rom, sizeof(mock_entities_rom));

    gb_write(&gb, hActiveEntityType, 0x05);
    dispatched_bank = 0;
    dispatched_addr = 0;
    ExecuteActiveEntityHandler(&gb, mock_handler_dispatch);
    assert(dispatched_bank == 0x0B);
    assert(dispatched_addr == 0x5678);
    assert(gb.rom_bank == 0x0B);
    assert(gb_read(&gb, wCurrentBank) == 0x0B);

    /* 6. ExecuteActiveEntityHandler_trampoline */
    gb_init(&gb);
    s_simple_cb_called = false;
    ExecuteActiveEntityHandler_trampoline(&gb, simple_test_cb);
    assert(s_simple_cb_called);
    assert(gb.rom_bank == 0x03);
    assert(gb_read(&gb, wCurrentBank) == 0x03);

    /* 7. AnimateEntities main loop */
    gb_init(&gb);
    gb_write(&gb, wBossAgonySFXCountdown, 1);
    gb_write(&gb, wDialogState, 0);
    gb_write(&gb, wC111, 5);
    gb_write(&gb, wLinkMotionState, 0);
    gb_write(&gb, hMapId, 0x02); /* < MAP_CAVE_B */
    gb_write(&gb, hFrameCounter, 2); /* slot = (2 & 3)*8 = 16 = 0x10 */

    /* Set two active entities: slot 1 and slot 7 */
    gb_write(&gb, wEntitiesStatusTable + 1, ENTITY_STATUS_ACTIVE);
    gb_write(&gb, wEntitiesStatusTable + 7, ENTITY_STATUS_ACTIVE);

    AnimateEntitiesCallbacks main_cbs = {
        .func_020_4303 = mock_func_020_4303,
        .func_020_6352 = mock_func_020_6352,
        .AnimateEntity = mock_animate_entity_slot,
    };

    test_func4303_called = 0;
    test_func6352_called = 0;
    test_animate_entity_count = 0;

    AnimateEntities(&gb, &main_cbs);

    assert(gb_read(&gb, wBossAgonySFXCountdown) == 0);
    assert(gb_read(&gb, hWaveSfx) == WAVE_SFX_BOSS_DEATH_CRY);
    assert(gb_read(&gb, wC1A8) == 5);
    assert(gb_read(&gb, wC111) == 4);
    assert(gb_read(&gb, wOAMNextAvailableSlot) == 0x10);
    assert(test_func4303_called == 1);
    assert(test_func6352_called == 1);
    assert(test_animate_entity_count == 2);
    /* Should loop down: slot 7 first, then slot 1 */
    assert(test_animated_slots[0] == 7);
    assert(test_animated_slots[1] == 1);

    /* Test Link passing out early return */
    gb_write(&gb, wLinkMotionState, LINK_MOTION_PASS_OUT);
    test_func4303_called = 0;
    AnimateEntities(&gb, &main_cbs);
    assert(test_func4303_called == 0); /* returned early */
}


static int test_render_cb_5d_called = 0;
static int test_render_cb_95_called = 0;
static uint16_t test_render_cb_entity = 0;

static void mock_func_015_795D(GBState *gb, uint16_t entity_index) {
    test_render_cb_5d_called++;
    test_render_cb_entity = entity_index;
    assert(gb->rom_bank == 0x15);
}

static void mock_func_015_7995(GBState *gb, uint16_t entity_index) {
    test_render_cb_95_called++;
    test_render_cb_entity = entity_index;
    assert(gb->rom_bank == 0x15);
}

static void test_entity_rendering_routines(void) {

    GBState gb;

    /* 1. SkipDisabledEntityDuringRoomTransition */
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 0);
    assert(SkipDisabledEntityDuringRoomTransition(&gb, 1) == false);

    gb_write(&gb, wRoomTransitionState, 1);
    gb_write(&gb, hActiveEntityPosX, 0x50);
    gb_write(&gb, hActiveEntityVisualPosY, 0x40);
    gb_write(&gb, wEntitiesPosXSignTable + 1, 0);
    gb_write(&gb, wEntitiesPosYSignTable + 1, 0);
    assert(SkipDisabledEntityDuringRoomTransition(&gb, 1) == false);

    /* Test X out of bounds: posX - 1 >= 0xC0 */
    gb_write(&gb, hActiveEntityPosX, 0xC1);
    assert(SkipDisabledEntityDuringRoomTransition(&gb, 1) == true);
    gb_write(&gb, hActiveEntityPosX, 0x50);

    /* Test Y out of bounds: posY - 1 >= 0x88 */
    gb_write(&gb, hActiveEntityVisualPosY, 0x89);
    assert(SkipDisabledEntityDuringRoomTransition(&gb, 1) == true);
    gb_write(&gb, hActiveEntityVisualPosY, 0x40);

    /* Test X sign */
    gb_write(&gb, wEntitiesPosXSignTable + 1, 1);
    assert(SkipDisabledEntityDuringRoomTransition(&gb, 1) == true);
    gb_write(&gb, wEntitiesPosXSignTable + 1, 0);

    /* Test Y sign */
    gb_write(&gb, wEntitiesPosYSignTable + 1, 1);
    assert(SkipDisabledEntityDuringRoomTransition(&gb, 1) == true);
    gb_write(&gb, wEntitiesPosYSignTable + 1, 0);

    /* 2. label_3C71 and label_3CD9 */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x07);
    gb.rom_bank = 0x15; /* in asm, caller sets bank $15 before label_3C71 */
    test_render_cb_95_called = 0;
    label_3C71(&gb, 3, mock_func_015_7995);
    assert(test_render_cb_95_called == 1);
    assert(test_render_cb_entity == 3);
    assert(gb.rom_bank == 0x07);

    test_render_cb_95_called = 0;
    label_3CD9(&gb, 4, mock_func_015_7995);
    assert(test_render_cb_95_called == 1);
    assert(test_render_cb_entity == 4);
    assert(gb.rom_bank == 0x07);

    /* 3. func_015_7964_trampoline */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x06);
    gb.rom_bank = 0x06;
    mock_cb_called = false;
    func_015_7964_trampoline(&gb, mock_cb_15);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x06);

    /* 4. RenderActiveEntitySpritesPair */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x03);
    gb.rom_bank = 0x03;
    gb_write(&gb, wActiveEntityIndex, 2);
    gb_write(&gb, wOAMNextAvailableSlot, 8);
    gb_write(&gb, hActiveEntitySpriteVariant, 0);
    gb_write(&gb, hActiveEntityVisualPosY, 0x30);
    gb_write(&gb, hActiveEntityPosX, 0x40);
    gb_write(&gb, wScreenShakeHorizontal, 0);
    gb_write(&gb, hActiveEntityTilesOffset, 0x10);
    gb_write(&gb, hActiveEntityFlipAttribute, 0);

    /* display list for variant 0: tile0=0x02, attr0=0x01, tile1=0x04, attr1=0x01 */
    static const uint8_t pair_display_list[4] = { 0x02, 0x01, 0x04, 0x01 };
    EntityRenderCallbacks render_cbs = {
        .func_015_795D = mock_func_015_795D,
        .func_015_7995 = mock_func_015_7995,
    };
    test_render_cb_5d_called = 0;
    test_render_cb_95_called = 0;

    RenderActiveEntitySpritesPair(&gb, pair_display_list, &render_cbs);

    assert(test_render_cb_5d_called == 1);
    assert(test_render_cb_95_called == 1);
    assert(test_render_cb_entity == 2);
    assert(gb.rom_bank == 0x03);

    /* OAM slot 8 is at wDynamicOAMBuffer + 8 = 0xC038 */
    /* Sprite 0: Y=0x30, X=0x40 (no flip, so x0 = 0 + 0x40 - 0 = 0x40), Tile=0x02+0x10=0x12, Attr=0x01 */
    assert(gb_read(&gb, wDynamicOAMBuffer + 8) == 0x30);
    assert(gb_read(&gb, wDynamicOAMBuffer + 9) == 0x40);
    assert(gb_read(&gb, wDynamicOAMBuffer + 10) == 0x12);
    assert(gb_read(&gb, wDynamicOAMBuffer + 11) == 0x01);
    /* Sprite 1: Y=0x30, X=0x40 + 8 = 0x48, Tile=0x04+0x10=0x14, Attr=0x01 */
    assert(gb_read(&gb, wDynamicOAMBuffer + 12) == 0x30);
    assert(gb_read(&gb, wDynamicOAMBuffer + 13) == 0x48);
    assert(gb_read(&gb, wDynamicOAMBuffer + 14) == 0x14);
    assert(gb_read(&gb, wDynamicOAMBuffer + 15) == 0x01);

    /* 5. RenderActiveEntitySprite */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x03);
    gb.rom_bank = 0x03;
    gb_write(&gb, wActiveEntityIndex, 1);
    gb_write(&gb, wOAMNextAvailableSlot, 0);
    gb_write(&gb, hActiveEntitySpriteVariant, 1);
    gb_write(&gb, hActiveEntityVisualPosY, 0x20);
    gb_write(&gb, hActiveEntityPosX, 0x18);
    gb_write(&gb, wScreenShakeHorizontal, 2);
    gb_write(&gb, hIsSideScrolling, 1); /* reduces visual Y by 4 to 0x1C */
    gb_write(&gb, hActiveEntityFlipAttribute, 0);

    /* display list: variant 0 (2 bytes), variant 1 (tile=0x30, attr=0x00) */
    static const uint8_t single_display_list[4] = { 0x10, 0x00, 0x30, 0x00 };
    test_render_cb_5d_called = 0;
    test_render_cb_95_called = 0;

    RenderActiveEntitySprite(&gb, single_display_list, &render_cbs);

    assert(test_render_cb_5d_called == 1);
    assert(test_render_cb_95_called == 1);
    assert(gb_read(&gb, hActiveEntityVisualPosY) == 0x1C);
    /* OAM slot 0 at wDynamicOAMBuffer + 0: Y=0x1C, X=0x18 + 4 - 2 = 0x1A, Tile=0x30, Attr=0x00 */
    assert(gb_read(&gb, wDynamicOAMBuffer + 0) == 0x1C);
    assert(gb_read(&gb, wDynamicOAMBuffer + 1) == 0x1A);
    assert(gb_read(&gb, wDynamicOAMBuffer + 2) == 0x30);
    assert(gb_read(&gb, wDynamicOAMBuffer + 3) == 0x00);

    /* 6. RenderActiveEntitySpritesRect and RenderActiveEntitySpritesRectUsingAllOAM */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x03);
    gb.rom_bank = 0x03;
    gb_write(&gb, wActiveEntityIndex, 0);
    gb_write(&gb, wOAMNextAvailableSlot, 0x10);
    gb_write(&gb, hActiveEntitySpriteVariant, 0);
    gb_write(&gb, hActiveEntityVisualPosY, 0x50);
    gb_write(&gb, hActiveEntityPosX, 0x60);
    gb_write(&gb, wScreenShakeHorizontal, 0);
    gb_write(&gb, hActiveEntityTilesOffset, 0x00);
    gb_write(&gb, hActiveEntityFlipAttribute, 0);

    /* Rect display list: 2 sprites, 4 bytes each: {relY, relX, tile, attr} */
    static const uint8_t rect_display_list[8] = {
        0x00, 0x00, 0x22, 0x02,
        0x08, 0x08, 0xFF, 0x03 /* tile 0xFF should be overridden to 0 */
    };
    test_render_cb_5d_called = 0;

    RenderActiveEntitySpritesRect(&gb, rect_display_list, 2, mock_func_015_795D);
    assert(test_render_cb_5d_called == 1);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x10) == 0x50);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x11) == 0x60);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x12) == 0x22);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x13) == 0x02);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x14) == 0x58);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x15) == 0x68);
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x16) == 0x00); /* overridden */
    assert(gb_read(&gb, wDynamicOAMBuffer + 0x17) == 0x03);

    /* RenderActiveEntitySpritesRectUsingAllOAM renders at wOAMBuffer (0xC000) */
    test_render_cb_5d_called = 0;
    RenderActiveEntitySpritesRectUsingAllOAM(&gb, rect_display_list, 1, mock_func_015_795D);
    assert(test_render_cb_5d_called == 1);
    assert(gb_read(&gb, wOAMBuffer + 0) == 0x50);
    assert(gb_read(&gb, wOAMBuffer + 1) == 0x60);
    assert(gb_read(&gb, wOAMBuffer + 2) == 0x22);
    assert(gb_read(&gb, wOAMBuffer + 3) == 0x02);
}


static int test_load_rupees_called = 0;
static int test_load_hearts_called = 0;
static void mock_load_rupees(GBState *gb) {
    test_load_rupees_called++;
    assert(gb->rom_bank == 0x02);
}
static void mock_load_hearts(GBState *gb) {
    test_load_hearts_called++;
    assert(gb->rom_bank == 0x02);
}

static uint16_t test_draw_slots_bc = 0;
static uint8_t test_draw_slots_e = 0;
static void mock_draw_slots(GBState *gb, uint16_t bc, uint8_t e) {
    assert(gb->rom_bank == 0x20);
    test_draw_slots_bc = bc;
    test_draw_slots_e = e;
}

static uint8_t test_given_item = 0;
static void mock_give_item(GBState *gb, uint8_t item) {
    assert(gb->rom_bank == 0x03);
    test_given_item = item;
}

static void test_boss_and_entity_init_trampolines(void) {

    GBState gb;

    /* 1. EntityInitMiniMoldorm_trampoline (Bank $04) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    EntityInitMiniMoldorm_trampoline(&gb, mock_cb_04);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 2. EntityInitMoldorm_trampoline (Bank $04) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    EntityInitMoldorm_trampoline(&gb, mock_cb_04);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 3. EntityInitFacade_trampoline (Bank $04) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    EntityInitFacade_trampoline(&gb, mock_cb_04);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 4. EntityInitSlimeEye_trampoline (Bank $04) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    EntityInitSlimeEye_trampoline(&gb, mock_cb_04);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 5. EntityInitGenie_trampoline (Bank $36) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    EntityInitGenie_trampoline(&gb, mock_cb_36);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 6. EntityInitSlimeEel_trampoline (Bank $05) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    EntityInitSlimeEel_trampoline(&gb, mock_cb_05);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 7. EntityInitDodongoSnake_trampoline (Bank $05) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    EntityInitDodongoSnake_trampoline(&gb, mock_cb_05);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 8. EntityInitHotHead_trampoline (Bank $05) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    EntityInitHotHead_trampoline(&gb, mock_cb_05);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 9. EntityInitEvilEagle_trampoline (Bank $05) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    EntityInitEvilEagle_trampoline(&gb, mock_cb_05);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 10. Entity67Handler_trampoline (Bank $05) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    Entity67Handler_trampoline(&gb, mock_cb_05);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 11. CheckPositionForMapTransition_trampoline (Bank $02) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x08);
    gb.rom_bank = 0x08;
    mock_cb_called = false;
    CheckPositionForMapTransition_trampoline(&gb, mock_cb_02);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x08);
    assert(gb_read(&gb, wCurrentBank) == 0x08);

    /* 12. GhiniMovement_trampoline (Bank $04) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    GhiniMovement_trampoline(&gb, mock_cb_04);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 13. SmashRock_trampoline (Bank $03) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x09);
    gb.rom_bank = 0x09;
    mock_cb_called = false;
    SmashRock_trampoline(&gb, mock_cb_03);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x09);

    /* 14. LoadHeartsAndRupeesCount (Bank $02) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x08);
    gb.rom_bank = 0x08;
    test_load_rupees_called = 0;
    test_load_hearts_called = 0;
    LoadHeartsAndRupeesCount(&gb, mock_load_rupees, mock_load_hearts);
    assert(test_load_rupees_called == 1);
    assert(test_load_hearts_called == 1);
    assert(gb.rom_bank == 0x08);

    /* 15. SpawnChestWithItemAndRestoreBank3 (Bank $02 -> Bank $03) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x07);
    gb.rom_bank = 0x07;
    mock_cb_called = false;
    SpawnChestWithItemAndRestoreBank3(&gb, mock_cb_02);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x03);
    assert(gb_read(&gb, wCurrentBank) == 0x03);

    /* 16. DrawABButtonSlots (Bank $20) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x08);
    gb.rom_bank = 0x08;
    test_draw_slots_bc = 0;
    test_draw_slots_e = 0;
    DrawABButtonSlots(&gb, mock_draw_slots);
    assert(test_draw_slots_bc == 0x0001);
    assert(test_draw_slots_e == 0xFF);
    assert(gb.rom_bank == 0x08);

    /* 17. GiveInventoryItem_trampoline (Bank $03) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x08);
    gb.rom_bank = 0x08;
    test_given_item = 0;
    GiveInventoryItem_trampoline(&gb, 0x0A, mock_give_item);
    assert(test_given_item == 0x0A);
    assert(gb.rom_bank == 0x08);

    /* 18. func_006_783C_trampoline (Bank $06 -> Bank $03) */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x07);
    gb.rom_bank = 0x07;
    mock_cb_called = false;
    func_006_783C_trampoline(&gb, mock_cb_06);
    assert(mock_cb_called);
    assert(gb.rom_bank == 0x03);
    assert(gb_read(&gb, wCurrentBank) == 0x03);

    /* 19. UnloadAllEntities */
    gb_init(&gb);
    for (int i = 0; i < MAX_ENTITIES; i++) {
        gb_write(&gb, wEntitiesStatusTable + i, (uint8_t)(i + 1));
    }
    UnloadAllEntities(&gb);
    for (int i = 0; i < MAX_ENTITIES; i++) {
        assert(gb_read(&gb, wEntitiesStatusTable + i) == 0);
    }
}


static int test_drop_cb_called = 0;
static uint16_t test_drop_cb_entity = 0;
static void mock_spawn_enemy_drop(GBState *gb, uint16_t entity_index) {
    test_drop_cb_called++;
    test_drop_cb_entity = entity_index;
    assert(gb->rom_bank == 0x03);
}

static int test_dialog_cb_called = 0;
static uint8_t test_dialog_id = 0;
static void mock_open_dialog(GBState *gb, uint8_t dialog_id) {
    (void)gb;
    test_dialog_cb_called++;
    test_dialog_id = dialog_id;
}

static void test_recoil_and_kill_enemy_routines(void) {

    GBState gb;

    /* 1. label_3E8E */
    gb_init(&gb);
    gb_write(&gb, wEntitiesPowerRecoilingTable + 1, 0); /* not recoiling */
    label_3E8E(&gb, 1);
    /* no vfx created, countdown remains 0 */
    assert(gb_read(&gb, wTranscientVfxCountdownTable + 0) == 0);

    /* Recoiling, frame matches */
    gb_write(&gb, wEntitiesPowerRecoilingTable + 1, 1);
    gb_write(&gb, hFrameCounter, 1); /* (1 ^ 1) & 3 == 0 */
    gb_write(&gb, hActiveEntityPosX, 0x44);
    gb_write(&gb, hActiveEntityVisualPosY, 0x55);
    label_3E8E(&gb, 1);
    assert(gb_read(&gb, hMultiPurpose0) == 0x44);
    assert(gb_read(&gb, hMultiPurpose1) == 0x55);
    /* VFX slot countdown set to 0x0F */
    assert(gb_read(&gb, wTranscientVfxCountdownTable + 0x0F) == 0x0F);

    /* 2. StopEntityRecoilOnCollision */
    gb_init(&gb);
    /* Horizontal dominant: vx = -4 (0xFC, abs=4), vy = 2, abs_vx > abs_vy */
    gb_write(&gb, wEntitiesRecoilVelocityX + 2, 0xFC);
    gb_write(&gb, wEntitiesRecoilVelocityY + 2, 0x02);
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + 2, 10);
    gb_write(&gb, wEntitiesCollisionsTable + 2, 0x01); /* right collision */
    StopEntityRecoilOnCollision(&gb, 2);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + 2) == 0);

    /* Vertical dominant: vx = 1, vy = -5 (0xFB, abs=5), abs_vy >= abs_vx */
    gb_write(&gb, wEntitiesRecoilVelocityX + 3, 0x01);
    gb_write(&gb, wEntitiesRecoilVelocityY + 3, 0xFB);
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + 3, 10);
    gb_write(&gb, wEntitiesCollisionsTable + 3, 0x04); /* top collision (mask 0x0C) */
    StopEntityRecoilOnCollision(&gb, 3);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + 3) == 0);

    /* No collision match */
    gb_write(&gb, wEntitiesRecoilVelocityX + 4, 0x05);
    gb_write(&gb, wEntitiesRecoilVelocityY + 4, 0x01);
    gb_write(&gb, wEntitiesIgnoreHitsCountdownTable + 4, 10);
    gb_write(&gb, wEntitiesCollisionsTable + 4, 0x04); /* only vertical bit set, but recoil is horizontal */
    StopEntityRecoilOnCollision(&gb, 4);
    assert(gb_read(&gb, wEntitiesIgnoreHitsCountdownTable + 4) == 10);

    /* 3. BossIntro */
    gb_init(&gb);
    gb_write(&gb, wRoomTransitionState, 0);
    gb_write(&gb, wInventoryAppearing, 0);
    gb_write(&gb, wBossIntroDelay, 2);
    gb_write(&gb, wDidBossIntro, 0);
    test_dialog_cb_called = 0;
    BossIntro(&gb, 0, mock_open_dialog);
    assert(gb_read(&gb, wBossIntroDelay) == 1);
    assert(test_dialog_cb_called == 0);

    BossIntro(&gb, 0, mock_open_dialog);
    assert(gb_read(&gb, wBossIntroDelay) == 0);
    assert(test_dialog_cb_called == 0);

    /* Now delay is 0, triggers boss intro */
    gb_write(&gb, wEntitiesOptions1Table + 0, 0x00); /* boss */
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, hActiveEntityType, 0x20); /* standard boss */
    gb_write(&gb, hMapId, MAP_BOTTLE_GROTTO); /* map 1 -> dialog 0xB4 */
    BossIntro(&gb, 0, mock_open_dialog);
    assert(gb_read(&gb, wDidBossIntro) == 1);
    assert(gb_read(&gb, wMusicTrackToPlay) == MUSIC_BOSS);
    assert(gb_read(&gb, hDefaultMusicTrackAlt) == MUSIC_BOSS);
    assert(test_dialog_cb_called == 1);
    assert(test_dialog_id == 0xB4);

    /* Desert Lanmola dialog */
    gb_write(&gb, wDidBossIntro, 0);
    gb_write(&gb, hActiveEntityType, ENTITY_DESERT_LANMOLA);
    test_dialog_cb_called = 0;
    BossIntro(&gb, 0, mock_open_dialog);
    assert(test_dialog_cb_called == 1);
    assert(test_dialog_id == Dialog0DA);

    /* Grim Creeper dialog */
    gb_write(&gb, wDidBossIntro, 0);
    gb_write(&gb, hActiveEntityType, ENTITY_GRIM_CREEPER);
    test_dialog_cb_called = 0;
    BossIntro(&gb, 0, mock_open_dialog);
    assert(test_dialog_cb_called == 1);
    assert(test_dialog_id == Dialog026);

    /* 4. DidKillEnemy & UnloadEntity */
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x07);
    gb.rom_bank = 0x07;
    gb_write(&gb, wEntitiesLoadOrderTable + 1, 0x02);
    gb_write(&gb, wEntitiesStatusTable + 1, 1);
    gb_write(&gb, wKillCount, 0);
    gb_write(&gb, hMapRoom, 0x2A);
    gb_write(&gb, wEntitiesClearedRooms + 0x2A, 0);
    test_drop_cb_called = 0;

    DidKillEnemy(&gb, 1, mock_spawn_enemy_drop);
    assert(test_drop_cb_called == 1);
    assert(test_drop_cb_entity == 1);
    assert(gb_read(&gb, wEnemyWasKilled) == 0x03);
    assert(gb_read(&gb, wKillCount) == 1);
    assert(gb_read(&gb, wKillOrder + 0) == 0x02);
    /* load_order 2 -> bit 1 << 2 = 4 */
    assert(gb_read(&gb, wEntitiesClearedRooms + 0x2A) == 0x04);
    assert(gb_read(&gb, wEntitiesStatusTable + 1) == 0); /* unloaded */
    assert(gb.rom_bank == 0x07);

    /* UnloadEntity and UnloadEntityAndReturn */
    gb_write(&gb, wEntitiesStatusTable + 5, 2);
    UnloadEntity(&gb, 5);
    assert(gb_read(&gb, wEntitiesStatusTable + 5) == 0);

    gb_write(&gb, wEntitiesStatusTable + 6, 3);
    UnloadEntityAndReturn(&gb, 6);
    assert(gb_read(&gb, wEntitiesStatusTable + 6) == 0);
}


static bool mock_recent_rooms_called = false;
static void mock_update_recent_rooms(GBState *gb) {
    (void)gb;
    mock_recent_rooms_called = true;
}

static uint8_t mock_spawn_entity_type = 0;
static uint16_t mock_spawn_new_entity(GBState *gb, uint8_t entity_type) {
    (void)gb;
    mock_spawn_entity_type = entity_type;
    return 3; /* return slot 3 */
}

static bool mock_config_entity_called = false;
static uint8_t mock_config_slot = 0;
static void mock_configure_new_entity(GBState *gb, uint8_t slot) {
    (void)gb;
    mock_config_entity_called = true;
    mock_config_slot = slot;
}

static bool mock_prep_pos_called = false;
static uint8_t mock_prep_slot = 0;
static void mock_prepare_entity_pos(GBState *gb, uint8_t slot) {
    (void)gb;
    mock_prep_pos_called = true;
    mock_prep_slot = slot;
}

static void test_load_entity_from_definition_and_room(void) {

    GBState gb;

    /* 1. LoadEntityFromDefinition - success */
    gb_init(&gb);
    gb_write(&gb, hMultiPurposeD, 0);
    gb_write(&gb, hMapRoom, 0x12);
    gb_write(&gb, wEntitiesClearedRooms + 0x12, 0);

    /* Definition at 0xD000: Y=0x30, X=0x04 -> byte0=0x34; Type=0x55 */
    gb_write(&gb, 0xD000, 0x34);
    gb_write(&gb, 0xD001, 0x55);
    uint16_t def_ptr = 0xD000;

    mock_config_entity_called = false;
    mock_prep_pos_called = false;
    uint8_t slot = LoadEntityFromDefinition(&gb, &def_ptr, mock_configure_new_entity, mock_prepare_entity_pos);

    assert(slot == 0);
    assert(def_ptr == 0xD002);
    assert(gb_read(&gb, wEntitiesStatusTable + 0) == ENTITY_STATUS_INIT);
    assert(gb_read(&gb, wEntitiesPosYTable + 0) == 0x40); /* 0x30 + 0x10 */
    assert(gb_read(&gb, wEntitiesPosXTable + 0) == 0x48); /* 0x40 + 0x08 */
    assert(gb_read(&gb, wEntitiesTypeTable + 0) == 0x55);
    assert(mock_config_entity_called && mock_config_slot == 0);
    assert(mock_prep_pos_called && mock_prep_slot == 0);
    assert(gb.rom_bank == BANK_OverworldEntitiesPointersTable);

    /* 2. LoadEntityFromDefinition - skipped because cleared */
    gb_init(&gb);
    gb_write(&gb, hMultiPurposeD, 2); /* order 2 -> bit 1 << 2 = 0x04 */
    gb_write(&gb, hMapRoom, 0x15);
    gb_write(&gb, wEntitiesClearedRooms + 0x15, 0x04);
    gb_write(&gb, 0xD000, 0x34);
    gb_write(&gb, 0xD001, 0x55);
    def_ptr = 0xD000;

    slot = LoadEntityFromDefinition(&gb, &def_ptr, NULL, NULL);
    assert(slot == 0xFF);
    assert(def_ptr == 0xD002);
    assert(gb_read(&gb, hMultiPurposeD) == 3);

    /* 3. LoadEntityFromDefinition - skipped because all slots full */
    gb_init(&gb);
    gb_write(&gb, hMultiPurposeD, 0);
    for (int i = 0; i < 16; i++) {
        gb_write(&gb, wEntitiesStatusTable + i, ENTITY_STATUS_ACTIVE);
    }
    gb_write(&gb, 0xD000, 0x34);
    gb_write(&gb, 0xD001, 0x55);
    def_ptr = 0xD000;

    slot = LoadEntityFromDefinition(&gb, &def_ptr, NULL, NULL);
    assert(slot == 0xFF);
    assert(def_ptr == 0xD002);
    assert(gb_read(&gb, hMultiPurposeD) == 1);

    /* 4. LoadRoomEntities - Overworld */
    gb_init(&gb);
    static uint8_t mock_ent_rom[0x4000 * 0x20];
    memset(mock_ent_rom, 0, sizeof(mock_ent_rom));
    gb_attach_rom(&gb, mock_ent_rom, sizeof(mock_ent_rom));

    gb_write(&gb, wCurrentBank, 1);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, hMapRoom, 0x04);

    /* OverworldEntitiesPointersTable ($4000 in bank $16): entry for room 4 ($4008) -> points to $4100 */
    size_t ptr_offset = (size_t)0x16 * 0x4000 + (OverworldEntitiesPointersTable + 0x08 - 0x4000);
    mock_ent_rom[ptr_offset] = 0x00;
    mock_ent_rom[ptr_offset + 1] = 0x41;

    /* Entity list at $4100: entity 1 (pos 0x22, type 0x10), end sentinel $FF */
    size_t list_offset = (size_t)0x16 * 0x4000 + (0x4100 - 0x4000);
    mock_ent_rom[list_offset + 0] = 0x22;
    mock_ent_rom[list_offset + 1] = 0x10;
    mock_ent_rom[list_offset + 2] = ENTITIES_END;

    mock_recent_rooms_called = false;
    LoadRoomEntities(&gb, mock_update_recent_rooms, NULL, NULL, NULL);

    assert(mock_recent_rooms_called);
    assert(gb_read(&gb, wEntitiesStatusTable + 0) == ENTITY_STATUS_INIT);
    assert(gb_read(&gb, wEntitiesTypeTable + 0) == 0x10);
    assert(gb.rom_bank == 1); /* restored */

    /* 5. LoadRoomEntities - Eagle's Tower wrecking ball */
    gb_init(&gb);
    gb_attach_rom(&gb, mock_ent_rom, sizeof(mock_ent_rom));
    gb_write(&gb, wCurrentBank, 1);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hMapId, MAP_EAGLES_TOWER);
    gb_write(&gb, hMapRoom, 0x06);
    gb_write(&gb, wWreckingBallRoom, 0x06);
    gb_write(&gb, wWreckingBallPosX, 0x50);
    gb_write(&gb, wWreckingBallPosY, 0x60);

    /* IndoorsBEntitiesPointersTable ($4400 in bank $16): entry for room 6 ($440C) -> points to $4200 */
    ptr_offset = (size_t)0x16 * 0x4000 + (IndoorsBEntitiesPointersTable + 0x0C - 0x4000);
    mock_ent_rom[ptr_offset] = 0x00;
    mock_ent_rom[ptr_offset + 1] = 0x42;
    list_offset = (size_t)0x16 * 0x4000 + (0x4200 - 0x4000);
    mock_ent_rom[list_offset] = ENTITIES_END;

    mock_spawn_entity_type = 0;
    LoadRoomEntities(&gb, NULL, mock_spawn_new_entity, NULL, NULL);

    assert(mock_spawn_entity_type == ENTITY_WRECKING_BALL);
    /* Wrecking ball placed at slot 3 */
    assert(gb_read(&gb, wEntitiesPosXTable + 3) == 0x50);
    assert(gb_read(&gb, wEntitiesPosYTable + 3) == 0x60);
    assert(gb_read(&gb, wEntitiesLoadOrderTable + 3) == 0xFF);
    assert(gb.rom_bank == 1);
}

#define RUN_ENTITY_TEST(fn, name) \
    do { \
        printf("[RUN ] %s\n", name); \
        fn(); \
        printf("[PASS] %s\n", name); \
    } while (0)

void run_entities_tests(void) {
    printf("[TEST] Entities (Bank 0)\n");
    RUN_ENTITY_TEST(test_load_entity_from_definition_and_room, "LoadEntityFromDefinitionAndRoom");
    RUN_ENTITY_TEST(test_is_zero, "IsZero");
    RUN_ENTITY_TEST(test_entity_countdowns, "EntityCountdowns");
    RUN_ENTITY_TEST(test_create_trading_item_entity, "CreateTradingItemEntity");
    RUN_ENTITY_TEST(test_spawn_entity_trampolines, "SpawnEntityTrampolines");
    RUN_ENTITY_TEST(test_animate_entities_trampolines, "AnimateEntitiesTrampolines");
    RUN_ENTITY_TEST(test_entities_batch_trampolines, "EntitiesBatchTrampolines");
    RUN_ENTITY_TEST(test_entities_batch_hitbox_and_collision_trampolines, "HitboxAndCollisionTrampolines");
    RUN_ENTITY_TEST(test_animate_entities_pipeline, "AnimateEntitiesPipeline");
    RUN_ENTITY_TEST(test_entity_rendering_routines, "EntityRenderingRoutines");
    RUN_ENTITY_TEST(test_boss_and_entity_init_trampolines, "BossAndEntityInitTrampolines");
    RUN_ENTITY_TEST(test_recoil_and_kill_enemy_routines, "RecoilAndKillEnemyRoutines");
    printf("[PASS] Entities (Bank 0)\n\n");
}
