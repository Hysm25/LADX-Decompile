#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "gb.h"
#include "home/link.h"
#include "constants/hardware.h"
#include "constants/memory.h"
#include "constants/gameplay.h"
#include "constants/sfx.h"
#include "constants/joypad.h"
#include "constants/inventory.h"

static int failures = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++; \
        } \
    } while (0)

static void test_disableMovement_and_playNoiseStairs(void) {
    GBState gb;

    /* test disableMovementInTransition directly */
    gb_init(&gb);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_DEFAULT);
    gb_write(&gb, wTransitionSequenceCounter, 0x04);
    gb_write(&gb, wC16C, 0x12);
    gb_write(&gb, wD478, 0x34);

    disableMovementInTransition(&gb);

    TEST_ASSERT(gb_read(&gb, wLinkMotionState) == LINK_MOTION_MAP_FADE_OUT, "Motion state not LINK_MOTION_MAP_FADE_OUT");
    TEST_ASSERT(gb_read(&gb, wTransitionSequenceCounter) == 0, "Transition counter not cleared");
    TEST_ASSERT(gb_read(&gb, wC16C) == 0, "wC16C not cleared");
    TEST_ASSERT(gb_read(&gb, wD478) == 0, "wD478 not cleared");

    /* test playNoiseStairs directly */
    gb_init(&gb);
    gb_write(&gb, hNoiseSfx, NOISE_SFX_NONE);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_DEFAULT);
    gb_write(&gb, wTransitionSequenceCounter, 0x08);

    playNoiseStairs(&gb);

    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_STAIRS, "Noise SFX not set to NOISE_SFX_STAIRS");
    TEST_ASSERT(gb_read(&gb, wLinkMotionState) == LINK_MOTION_MAP_FADE_OUT, "Motion state not set");
    TEST_ASSERT(gb_read(&gb, wTransitionSequenceCounter) == 0, "Transition counter not cleared");
}

static void test_fade_out_transitions(void) {
    GBState gb;

    /* Test ApplyMapFadeOutTransitionWithNoise */
    gb_init(&gb);
    gb_write(&gb, hMusicFadeOutTimer, 0);
    gb_write(&gb, hNoiseSfx, NOISE_SFX_NONE);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_DEFAULT);
    gb_write(&gb, wTransitionSequenceCounter, 5);
    gb_write(&gb, wC16C, 5);
    gb_write(&gb, wD478, 5);

    ApplyMapFadeOutTransitionWithNoise(&gb);

    TEST_ASSERT(gb_read(&gb, hMusicFadeOutTimer) == 0x30, "Fade out timer not set to 0x30");
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_STAIRS, "Noise SFX not set to NOISE_SFX_STAIRS");
    TEST_ASSERT(gb_read(&gb, wLinkMotionState) == LINK_MOTION_MAP_FADE_OUT, "Motion state not LINK_MOTION_MAP_FADE_OUT");
    TEST_ASSERT(gb_read(&gb, wTransitionSequenceCounter) == 0, "Transition counter not reset");
    TEST_ASSERT(gb_read(&gb, wC16C) == 0, "wC16C not reset");
    TEST_ASSERT(gb_read(&gb, wD478) == 0, "wD478 not reset");

    /* Test ApplyMapFadeOutTransition */
    gb_init(&gb);
    gb_write(&gb, hMusicFadeOutTimer, 0);
    gb_write(&gb, hNoiseSfx, NOISE_SFX_NONE);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_DEFAULT);

    ApplyMapFadeOutTransition(&gb);

    TEST_ASSERT(gb_read(&gb, hMusicFadeOutTimer) == 0x30, "Fade out timer not set to 0x30");
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_NONE, "Noise SFX should not be modified");
    TEST_ASSERT(gb_read(&gb, wLinkMotionState) == LINK_MOTION_MAP_FADE_OUT, "Motion state not LINK_MOTION_MAP_FADE_OUT");

    /* Test ApplyMapFadeOutTransitionWithSound (Indoors & warp category 1) */
    gb_init(&gb);
    gb_write(&gb, wWarp0MapCategory, 1);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hContinueMusicAfterWarp, 0);
    gb_write(&gb, hMusicFadeOutTimer, 0);
    gb_write(&gb, hNoiseSfx, NOISE_SFX_NONE);

    ApplyMapFadeOutTransitionWithSound(&gb);

    TEST_ASSERT(gb_read(&gb, hContinueMusicAfterWarp) == 1, "hContinueMusicAfterWarp not set to 1");
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_STAIRS, "Noise SFX not set to NOISE_SFX_STAIRS");
    TEST_ASSERT(gb_read(&gb, hMusicFadeOutTimer) == 0, "hMusicFadeOutTimer modified when music continues");
    TEST_ASSERT(gb_read(&gb, wLinkMotionState) == LINK_MOTION_MAP_FADE_OUT, "Motion state not set");

    /* Test ApplyMapFadeOutTransitionWithSound (Outdoors -> should use noise with fade out) */
    gb_init(&gb);
    gb_write(&gb, wWarp0MapCategory, 1);
    gb_write(&gb, wIsIndoor, 0);
    gb_write(&gb, hContinueMusicAfterWarp, 0);
    gb_write(&gb, hMusicFadeOutTimer, 0);
    gb_write(&gb, hNoiseSfx, NOISE_SFX_NONE);

    ApplyMapFadeOutTransitionWithSound(&gb);

    TEST_ASSERT(gb_read(&gb, hContinueMusicAfterWarp) == 0, "hContinueMusicAfterWarp should not be set outdoors");
    TEST_ASSERT(gb_read(&gb, hMusicFadeOutTimer) == 0x30, "Fade out timer not set to 0x30 outdoors");
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_STAIRS, "Noise SFX not set outdoors");

    /* Test ApplyMapFadeOutTransitionWithSound (Indoors but different warp category) */
    gb_init(&gb);
    gb_write(&gb, wWarp0MapCategory, 2);
    gb_write(&gb, wIsIndoor, 1);
    gb_write(&gb, hContinueMusicAfterWarp, 0);
    gb_write(&gb, hMusicFadeOutTimer, 0);

    ApplyMapFadeOutTransitionWithSound(&gb);

    TEST_ASSERT(gb_read(&gb, hContinueMusicAfterWarp) == 0, "hContinueMusicAfterWarp should not be set for category 2");
    TEST_ASSERT(gb_read(&gb, hMusicFadeOutTimer) == 0x30, "Fade out timer not set to 0x30 for category 2");
}

static void test_resets_and_position(void) {
    GBState gb;

    /* ResetPegasusBoots */
    gb_init(&gb);
    gb_write(&gb, wPegasusBootsChargeMeter, 0x15);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0x01);
    gb_write(&gb, wIsUsingSpinAttack, 0x20);
    gb_write(&gb, wSwordCharge, 0x28);

    ResetPegasusBoots(&gb);

    TEST_ASSERT(gb_read(&gb, wPegasusBootsChargeMeter) == 0, "Pegasus boots meter not cleared");
    TEST_ASSERT(gb_read(&gb, wIsRunningWithPegasusBoots) == 0, "Running with boots flag not cleared");
    TEST_ASSERT(gb_read(&gb, wIsUsingSpinAttack) == 0x20, "Spin attack modified by ResetPegasusBoots");
    TEST_ASSERT(gb_read(&gb, wSwordCharge) == 0x28, "Sword charge modified by ResetPegasusBoots");

    /* ResetSpinAttack */
    ResetSpinAttack(&gb);

    TEST_ASSERT(gb_read(&gb, wIsUsingSpinAttack) == 0, "Spin attack flag not cleared");
    TEST_ASSERT(gb_read(&gb, wSwordCharge) == 0, "Sword charge not cleared");
    TEST_ASSERT(gb_read(&gb, wPegasusBootsChargeMeter) == 0, "Pegasus boots meter not cleared by spin reset");
    TEST_ASSERT(gb_read(&gb, wIsRunningWithPegasusBoots) == 0, "Running flag not cleared by spin reset");

    /* CopyLinkFinalPositionToPosition */
    gb_init(&gb);
    gb_write(&gb, hLinkFinalPositionX, 0x45);
    gb_write(&gb, hLinkFinalPositionY, 0x82);
    gb_write(&gb, hLinkPositionX, 0x00);
    gb_write(&gb, hLinkPositionY, 0x00);

    CopyLinkFinalPositionToPosition(&gb);

    TEST_ASSERT(gb_read(&gb, hLinkPositionX) == 0x45, "hLinkPositionX mismatch");
    TEST_ASSERT(gb_read(&gb, hLinkPositionY) == 0x82, "hLinkPositionY mismatch");
}

static int anim_called = 0;
static void hook_walk_anim(GBState *gb) {
    anim_called++;
    TEST_ASSERT(gb->rom_bank == 0x02, "ROM bank not switched to 0x02 during anim update");
}

static void test_update_link_walking_animation_trampoline(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, wCurrentBank, 0x07);
    gb.rom_bank = 0x07;

    anim_called = 0;
    UpdateLinkWalkingAnimation_trampoline(&gb, hook_walk_anim);

    TEST_ASSERT(anim_called == 1, "Animation callback not called");
    TEST_ASSERT(gb.rom_bank == 0x07, "ROM bank not restored to wCurrentBank (0x07)");
}


#include "constants/directions.h"
#include "constants/entities.h"
#include "constants/link.h"
#include "constants/vfx.h"

static void test_clear_link_position_increment(void) {
    GBState gb;
    gb_init(&gb);
    gb_write(&gb, hLinkSpeedX, 0x20);
    gb_write(&gb, hLinkSpeedY, 0x10);

    ClearLinkPositionIncrement(&gb);

    TEST_ASSERT(gb_read(&gb, hLinkSpeedX) == 0, "hLinkSpeedX not cleared");
    TEST_ASSERT(gb_read(&gb, hLinkSpeedY) == 0, "hLinkSpeedY not cleared");
}

static void test_use_pegasus_boots(void) {
    GBState gb;

    /* 1. In air -> ignored */
    gb_init(&gb);
    gb_write(&gb, wIsLinkInTheAir, 1);
    UsePegasusBoots(&gb);
    TEST_ASSERT(gb_read(&gb, wPegasusBootsChargeMeter) == 0, "Boots charged while in air");

    /* 2. Side-scrolling vertical movement -> ignored */
    gb_init(&gb);
    gb_write(&gb, hIsSideScrolling, 1);
    gb_write(&gb, hLinkDirection, DIRECTION_UP);
    UsePegasusBoots(&gb);
    TEST_ASSERT(gb_read(&gb, wPegasusBootsChargeMeter) == 0, "Boots charged moving vertically in side-scroller");

    /* 3. Normal charge progression */
    gb_init(&gb);
    gb_write(&gb, wPegasusBootsChargeMeter, 0x1F);
    gb_write(&gb, hLinkDirection, DIRECTION_RIGHT);
    UsePegasusBoots(&gb);

    TEST_ASSERT(gb_read(&gb, wPegasusBootsChargeMeter) == 0x20, "Charge meter not incremented to 0x20");
    TEST_ASSERT(gb_read(&gb, wIsRunningWithPegasusBoots) == 0x20, "wIsRunningWithPegasusBoots not set to 0x20");
    TEST_ASSERT(gb_read(&gb, hLinkSpeedX) == 32, "LinkSpeedX not set to 32 for DIRECTION_RIGHT");
    TEST_ASSERT(gb_read(&gb, hLinkSpeedY) == 0, "LinkSpeedY not 0 for DIRECTION_RIGHT");
}

static void test_display_transient_vfx_for_link_running(void) {
    GBState gb;

    /* 1. Frame counter not aligned to 8 -> ignored */
    gb_init(&gb);
    gb_write(&gb, hFrameCounter, 3);
    DisplayTransientVfxForLinkRunning(&gb);
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == 0, "VFX triggered off 8-frame boundary");

    /* 2. Ground running dust */
    gb_init(&gb);
    gb_write(&gb, hFrameCounter, 8);
    gb_write(&gb, hLinkPositionX, 50);
    gb_write(&gb, hLinkPositionY, 60);
    DisplayTransientVfxForLinkRunning(&gb);
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_FOOTSTEP, "Footstep SFX not played on ground running");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose0) == 50, "hMultiPurpose0 mismatch");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose1) == 66, "hMultiPurpose1 mismatch (Y + 6)");

    /* 3. Shallow water splash */
    gb_init(&gb);
    gb_write(&gb, hFrameCounter, 0);
    gb_write(&gb, wLinkObjectPhysics, OBJ_PHYSICS_SHALLOW_WATER);
    gb_write(&gb, hLinkPositionX, 70);
    gb_write(&gb, hLinkPositionY, 80);
    DisplayTransientVfxForLinkRunning(&gb);
    TEST_ASSERT(gb_read(&gb, hJingle) == JINGLE_WATER_SPLASH, "Water splash jingle not played on shallow water");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose1) == 80, "hMultiPurpose1 mismatch for water");
}

static int mock_alert_moblins_calls = 0;
static void mock_alert_moblins(GBState *gb) {
    (void)gb;
    mock_alert_moblins_calls++;
}

static void test_check_items_sword_collision(void) {
    GBState gb;

    /* 1. wC16D == 0 -> no collision response */
    gb_init(&gb);
    gb_write(&gb, wC16D, 0);
    CheckItemsSwordCollision(&gb, 0x90, mock_alert_moblins);
    TEST_ASSERT(gb_read(&gb, wC1C4) == 0, "wC1C4 set when wC16D == 0");

    /* 2. Wall clink SFX (0x90 physics) */
    gb_init(&gb);
    gb_write(&gb, wC16D, 0x05);
    gb_write(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, hLinkPositionX, 100);
    gb_write(&gb, hLinkPositionY, 100);
    mock_alert_moblins_calls = 0;

    CheckItemsSwordCollision(&gb, 0x92, mock_alert_moblins);
    TEST_ASSERT(mock_alert_moblins_calls == 1, "AlertSwordMoblins not called");
    TEST_ASSERT(gb_read(&gb, wC1C4) == 0x10, "wC1C4 not set to 0x10");
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_CLINK, "Noise SFX not set to NOISE_SFX_CLINK");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose0) == 100 + 0x12, "MultiPurpose0 mismatch for DIRECTION_RIGHT");

    /* 3. Sword poking jingle for non-0x90 */
    gb_init(&gb);
    gb_write(&gb, wC16D, 0x05);
    gb_write(&gb, hLinkDirection, DIRECTION_LEFT);
    CheckItemsSwordCollision(&gb, 0xD0, mock_alert_moblins);
    TEST_ASSERT(gb_read(&gb, hJingle) == JINGLE_SWORD_POKING, "Jingle not set to JINGLE_SWORD_POKING");
}

static uint8_t mock_get_physics(GBState *gb, uint8_t obj, uint8_t indoor) {
    (void)gb; (void)indoor;
    if (obj == 0xD3) return 0x00; /* Bush */
    if (obj == 0x90) return 0x90; /* Wall */
    return 0x00;
}

static int mock_smash_calls = 0;
static void mock_start_smashing(GBState *gb, uint8_t idx) {
    (void)gb; (void)idx;
    mock_smash_calls++;
}

static void test_check_static_sword_collision(void) {
    GBState gb;

    /* 1. Cut bush (outdoor obj 0xD3) */
    gb_init(&gb);
    gb_write(&gb, hLinkPositionX, 0x40);
    gb_write(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wIsIndoor, 0);

    /* Address for room object under entity */
    /* x = (0x40 + 0x16 - 8) & 0xF0 = 0x40. c = 0x04 */
    /* y = (0x40 + 0x08 - 16) & 0xF0 = 0x30 */
    /* e = y | c = 0x34 */
    gb_write(&gb, wRoomObjects + 0x34, 0xD3); /* Bush */

    mock_smash_calls = 0;
    CheckStaticSwordCollision(&gb, mock_get_physics, NULL, mock_start_smashing, NULL);

    TEST_ASSERT(gb_read(&gb, hObjectUnderEntity) == 0xD3, "hObjectUnderEntity not 0xD3");
    TEST_ASSERT(gb_read(&gb, hActiveEntitySpriteVariant) == 1, "Sprite variant not 1 for bush");
    TEST_ASSERT(mock_smash_calls == 1, "LiftableRockStartSmashingAnimation not called");
}

static void test_check_static_sword_collision_trampoline(void) {
    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wC1C4, 1); /* Quick return */
    gb.rom_bank = 0x01;

    CheckStaticSwordCollision_trampoline(&gb, NULL, NULL, NULL, NULL);
    TEST_ASSERT(gb.rom_bank == 0x02, "ROM bank not switched to 0x02 by trampoline");
}


static int mock_bank20_calls = 0;
static void mock_bank20_func(GBState *gb) {
    (void)gb;
    mock_bank20_calls++;
}

static void test_func_1819_and_1828(void) {
    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x05);
    gb.rom_bank = 0x05;

    mock_bank20_calls = 0;
    func_1819(&gb, mock_bank20_func);
    TEST_ASSERT(mock_bank20_calls == 1, "func_1819 callback not called");
    TEST_ASSERT(gb.rom_bank == 0x05, "ROM bank not restored after func_1819");

    mock_bank20_calls = 0;
    func_1828(&gb, mock_bank20_func);
    TEST_ASSERT(mock_bank20_calls == 1, "func_1828 callback not called");
    TEST_ASSERT(gb.rom_bank == 0x05, "ROM bank not restored after func_1828");
}

static void test_func_1a22_and_1a39(void) {
    GBState gb;
    gb_init(&gb);
    gb_write(&gb, wCurrentBank, 0x03);
    gb.rom_bank = 0x03;

    mock_bank20_calls = 0;
    func_1A22(&gb, mock_bank20_func, mock_bank20_func);
    TEST_ASSERT(mock_bank20_calls == 2, "func_1A22 callbacks not called");
    TEST_ASSERT(gb.rom_bank == 0x03, "ROM bank not restored after func_1A22");

    mock_bank20_calls = 0;
    func_1A39(&gb, mock_bank20_func, mock_bank20_func);
    TEST_ASSERT(mock_bank20_calls == 2, "func_1A39 callbacks not called");
    TEST_ASSERT(gb.rom_bank == 0x03, "ROM bank not restored after func_1A39");
}

static void test_apply_link_motion_state(void) {
    GBState gb;

    /* 1. Swimming -> returns immediately */
    gb_init(&gb);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_SWIMMING);
    gb_write(&gb, wC16A, 1);
    ApplyLinkMotionState(&gb, NULL, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, hMultiPurpose0) == 0, "MultiPurpose0 modified while swimming");

    /* 2. Sword swing draw state */
    gb_init(&gb);
    gb_write(&gb, wC16A, 1);
    gb_write(&gb, wC145, 10);
    gb_write(&gb, wC13B, 20);
    gb_write(&gb, hLinkPositionX, 30);
    gb_write(&gb, hLinkPositionY, 40);
    gb_write(&gb, wSwordCharge, MAX_SWORD_CHARGE);
    gb_write(&gb, hFrameCounter, 0x04); /* (4 << 2) & 0x10 = 0x10 */
    gb_write(&gb, wSwordDirection, DIRECTION_UP);

    ApplyLinkMotionState(&gb, NULL, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, hMultiPurpose0) == 30, "MultiPurpose0 mismatch (10+20)");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose1) == 30, "MultiPurpose1 mismatch (PosX)");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose2) == DIRECTION_UP, "MultiPurpose2 mismatch (SwordDir)");
    TEST_ASSERT(gb_read(&gb, hMultiPurpose3) == 0x10, "MultiPurpose3 sword charge flash mismatch");

    /* 3. Magic rod attack step */
    gb_init(&gb);
    gb_write(&gb, wLinkAttackStepAnimationCountdown, 0x8C); /* bit 7 set, duration 0x0C */
    gb_write(&gb, hLinkDirection, DIRECTION_RIGHT);
    ApplyLinkMotionState(&gb, NULL, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, hNoiseSfx) == NOISE_SFX_MAGIC_ROD, "Magic rod noise SFX not played");
    TEST_ASSERT(gb_read(&gb, wEntitiesTypeTable) == ENTITY_MAGIC_ROD_FIREBALL, "Fireball not spawned");
    TEST_ASSERT(gb_read(&gb, wLinkAttackStepAnimationCountdown) == 0x8C, "Attack countdown not restored");
}

static void test_set_spawn_location_and_label_19da(void) {
    GBState gb;
    gb_init(&gb);

    /* Test label_19DA */
    gb_write(&gb, hLinkDirection, DIRECTION_LEFT);
    label_19DA(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkDirection) == 0, "hLinkDirection not reset to 0 by label_19DA");

    /* Test SetSpawnLocation */
    uint8_t warp_data[5] = { 0x01, 0x02, 0x03, 0x40, 0x50 };
    for (int i = 0; i < 5; i++) {
        gb_write(&gb, 0xD401 + i, warp_data[i]);
    }
    gb_write(&gb, wIndoorRoom, 0x2A);

    SetSpawnLocation(&gb, 0xD401);
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT(gb_read(&gb, wSpawnLocationData + i) == warp_data[i], "Spawn location warp data mismatch");
    }
    TEST_ASSERT(gb_read(&gb, wSpawnLocationData + 5) == 0x2A, "Spawn location indoor room mismatch");
}

static void test_link_motion_map_fade_in_handler(void) {
    GBState gb;

    /* 1. Manbo out transition (wD474 != 0) */
    gb_init(&gb);
    gb_write(&gb, wD474, 1);
    LinkMotionMapFadeInHandler(&gb, NULL, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, wD474) == 0, "wD474 not cleared");
    TEST_ASSERT(gb_read(&gb, wTransitionGfx) == TRANSITION_GFX_MANBO_OUT, "Transition GFX not MANBO_OUT");
    TEST_ASSERT(gb_read(&gb, wTransitionSequenceCounter) == 4, "Transition counter not set to 4");
    TEST_ASSERT(gb_read(&gb, wLinkMotionState) == LINK_MOTION_DEFAULT, "Link motion not set to DEFAULT");

    /* 2. Did steal item dialog trigger */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write(&gb, wDidStealItem, 1);
    LinkMotionMapFadeInHandler(&gb, NULL, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, wDidStealItem) == 0, "wDidStealItem not cleared");
    TEST_ASSERT(gb_read(&gb, wDialogIndex) == 0x36, "Dialog 0x36 not opened on thief return");
}

static void test_link_motion_map_fade_out_handler(void) {
    GBState gb;

    /* 1. wC3C9 != 0 -> trigger fade-out transition */
    gb_init(&gb);
    gb_write(&gb, wC3C9, 1);
    LinkMotionMapFadeOutHandler(&gb, NULL, NULL, NULL, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, wC3C9) == 0, "wC3C9 not cleared");
    TEST_ASSERT(gb_read(&gb, hMusicFadeOutTimer) == 0x30, "Fade out music timer not set to 0x30");

    /* 2. Full fade out with outdoor warp tile match */
    gb_init(&gb);
    gb_write(&gb, wTransitionSequenceCounter, 4);
    gb_write(&gb, wIsIndoor, 0); /* Outdoor */
    gb_write(&gb, hLinkPositionX, 0x20);
    gb_write(&gb, hLinkPositionY, 0x38);
    /* Link tile = ((0x20 >> 4) & 0x0F) | ((0x38 - 8) & 0xF0) = 0x02 | 0x30 = 0x32 */
    gb_write(&gb, wWarpPositions + 1, 0x32); /* Matches warp 1 */

    /* Setup warp 1 struct at wWarp0MapCategory + 5 */
    uint16_t warp1 = wWarp0MapCategory + 5;
    gb_write(&gb, warp1 + 0, 0x01); /* Indoor */
    gb_write(&gb, warp1 + 1, 0x02); /* MapId */
    gb_write(&gb, warp1 + 2, 0x03); /* MapRoom */
    gb_write(&gb, warp1 + 3, 0x40); /* PosX */
    gb_write(&gb, warp1 + 4, 0x50); /* PosY */

    LinkMotionMapFadeOutHandler(&gb, NULL, NULL, NULL, NULL, NULL);

    TEST_ASSERT(gb_read(&gb, wGameplayType) == GAMEPLAY_WORLD, "Gameplay type not GAMEPLAY_WORLD");
    TEST_ASSERT(gb_read(&gb, wIsIndoor) == 0x01, "wIsIndoor not set from warp struct");
    TEST_ASSERT(gb_read(&gb, hMapId) == 0x02, "hMapId mismatch");
    TEST_ASSERT(gb_read(&gb, hMapRoom) == 0x03, "hMapRoom mismatch");
    TEST_ASSERT(gb_read(&gb, wMapEntrancePositionX) == 0x40, "MapEntrancePositionX mismatch");
    TEST_ASSERT(gb_read(&gb, wMapEntrancePositionY) == 0x50, "MapEntrancePositionY mismatch");
}

static void test_update_link_walking_animation(void) {
    GBState gb;

    /* 1. Walking no shield, facing RIGHT */
    gb_init(&gb);
    gb_write(&gb, hLinkDirection, DIRECTION_RIGHT);
    gb_write(&gb, wConsecutiveStepsCount, 0); /* standing */
    UpdateLinkWalkingAnimation(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_STANDING_RIGHT, "Animation not STANDING_RIGHT");

    gb_write(&gb, wConsecutiveStepsCount, 8); /* walking (8/8 % 2 = 1) */
    UpdateLinkWalkingAnimation(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_WALKING_RIGHT, "Animation not WALKING_RIGHT");

    /* 2. Carrying default shield, facing UP */
    gb_init(&gb);
    gb_write(&gb, hLinkDirection, DIRECTION_UP);
    gb_write(&gb, wHasMirrorShield, 1);
    gb_write(&gb, wConsecutiveStepsCount, 0);
    UpdateLinkWalkingAnimation(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_STANDING_SHIELD_UP, "Animation not STANDING_SHIELD_UP");

    /* 3. Using default shield, facing DOWN */
    gb_init(&gb);
    gb_write(&gb, hLinkDirection, DIRECTION_DOWN);
    gb_write(&gb, wHasMirrorShield, 1);
    gb_write(&gb, wIsUsingShield, 1);
    gb_write(&gb, wConsecutiveStepsCount, 0);
    UpdateLinkWalkingAnimation(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_STANDING_SHIELD_USE_DOWN, "Animation not STANDING_SHIELD_USE_DOWN");

    /* 4. Lifting object */
    gb_init(&gb);
    gb_write(&gb, hLinkDirection, DIRECTION_LEFT);
    gb_write(&gb, wIsCarryingLiftedObject, 1);
    gb_write(&gb, wConsecutiveStepsCount, 0);
    UpdateLinkWalkingAnimation(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_STANDING_LIFTING_LEFT, "Animation not STANDING_LIFTING_LEFT");

    /* 5. Swimming */
    gb_init(&gb);
    gb_write(&gb, wLinkMotionState, LINK_MOTION_SWIMMING);
    gb_write(&gb, hLinkDirection, DIRECTION_DOWN);
    gb_write(&gb, wConsecutiveStepsCount, 0);
    UpdateLinkWalkingAnimation(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkAnimationState) == LINK_ANIMATION_STATE_HOLD_SWIMMING_1_DOWN, "Animation not HOLD_SWIMMING_1_DOWN");
}


static bool mock_reveal_called = false;
static void mock_reveal_object(GBState *gb) {
    (void)gb;
    mock_reveal_called = true;
}

static uint8_t mock_spawn_proj(GBState *gb, uint8_t entity_type) {
    (void)gb;
    (void)entity_type;
    return 2; /* slot 2 */
}

static bool mock_5795_called = false;
static void mock_func_003_5795(GBState *gb) {
    (void)gb;
    mock_5795_called = true;
}

static uint8_t mock_get_physics_flags(GBState *gb, uint8_t is_indoor, uint8_t obj) {
    (void)gb;
    (void)is_indoor;
    if (obj == 0x20 || obj == 0x8E || obj == 0x5C) return 0x15;
    return 0x00;
}

static void test_compute_link_position(void) {
    GBState gb;
    gb_init(&gb);

    /* 1. Horizontal speed positive */
    gb_write(&gb, hLinkPositionX, 0x50);
    gb_write(&gb, hLinkSpeedX, 0x18); /* speed = 1.5, swap = 0x81, swap&0xF0 = 0x80 */
    gb_write(&gb, wC11A, 0x00);

    ComputeLinkPosition(&gb, 0);

    /* acc becomes 0x80 (no carry), int_speed = 1 -> new_pos = 0x51 */
    TEST_ASSERT(gb_read(&gb, wC11A) == 0x80, "Horizontal subpixel accumulator mismatch");
    TEST_ASSERT(gb_read(&gb, hLinkPositionX) == 0x51, "Horizontal position mismatch");

    /* Second step: acc overflows (0x80 + 0x80 = 0x100 -> carry = 1), int_speed = 1 -> delta = 2 -> pos = 0x53 */
    ComputeLinkPosition(&gb, 0);
    TEST_ASSERT(gb_read(&gb, wC11A) == 0x00, "Horizontal subpixel accumulator overflow mismatch");
    TEST_ASSERT(gb_read(&gb, hLinkPositionX) == 0x53, "Horizontal position with carry mismatch");

    /* 2. UpdateFinalLinkPosition calls vertical and horizontal */
    gb_write(&gb, wInventoryAppearing, 0);
    gb_write(&gb, hLinkPositionY, 0x40);
    gb_write(&gb, hLinkSpeedY, 0x10); /* speed = 1.0 */
    gb_write(&gb, wC11B, 0x00);
    UpdateFinalLinkPosition(&gb);
    TEST_ASSERT(gb_read(&gb, hLinkPositionY) == 0x41, "Vertical position update mismatch");
}

static void test_func_21e1_z_velocity(void) {
    GBState gb;
    gb_init(&gb);

    gb_write(&gb, hLinkPositionZ, 0x10);
    gb_write(&gb, hLinkVelocityZ, 0x20); /* 2.0 */
    gb_write(&gb, wC149, 0x00);

    func_21E1(&gb);

    TEST_ASSERT(gb_read(&gb, hLinkPositionZ) == 0x12, "Z position integration mismatch");
}

static void test_label_2183_and_func_2165(void) {
    GBState gb;
    gb_init(&gb);

    mock_reveal_called = false;
    mock_5795_called = false;

    gb_write(&gb, hMultiPurpose0, 0x8E);
    gb_write(&gb, hMultiPurposeE, 0x01);
    gb_write(&gb, hLinkDirection, DIRECTION_UP);

    func_2165(&gb, mock_reveal_object, mock_spawn_proj, mock_func_003_5795);

    TEST_ASSERT(mock_reveal_called, "RevealObject not called in func_2165");
    TEST_ASSERT(mock_5795_called, "func_003_5795 not called in label_2183");
    TEST_ASSERT(gb_read(&gb, hObjectUnderEntity) == 0x8E, "hObjectUnderEntity mismatch");
    TEST_ASSERT(gb_read(&gb, wC15D) == DIRECTION_UP, "wC15D mismatch");
    TEST_ASSERT(gb_read(&gb, hWaveSfx) == WAVE_SFX_LIFT_UP, "Wave SFX mismatch");
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wEntitiesStatusTable + 2)) == 0x07, "Entity status mismatch");
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wEntitiesSpriteVariantTable + 2)) == 0x01, "Variant mismatch");
}

static void test_label_1f69_interactive_motion(void) {
    GBState gb;
    gb_init(&gb);

    /* 1. Motion state not default -> returns early */
    gb_write(&gb, wLinkMotionState, LINK_MOTION_MAP_FADE_OUT);
    gb_write(&gb, wPullCounter, 0x05);
    label_1F69(&gb, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, wPullCounter) == 0x05, "label_1F69 did not return on non-default motion");

    /* 2. Facing UP, object 0x20 in front, power bracelet equipped and pull button pressed */
    gb_write(&gb, wLinkMotionState, LINK_MOTION_DEFAULT);
    gb_write(&gb, wIsRunningWithPegasusBoots, 0);
    gb_write(&gb, wIsCarryingLiftedObject, 0);
    gb_write(&gb, hLinkPositionZ, 0);
    gb_write(&gb, hLinkDirection, DIRECTION_UP);
    gb_write(&gb, hLinkPositionX, 0x40);
    gb_write(&gb, hLinkPositionY, 0x40);

    /* SwordArea: X: 0x40 + 0x08 - 8 = 0x40. c = 0x04.
       Y: 0x40 + 0x05 - 0x10 = 0x35 -> 0x30.
       mp1 = 0x30 | 0x04 = 0x34. Room object at wRoomObjects + 0x34 (0xD711 + 0x34 = 0xD745) */
    gb_write(&gb, (uint16_t)(wRoomObjects + 0x34), 0x20);

    /* Power bracelet in A button slot (index 1) */
    gb_write(&gb, (uint16_t)(wInventoryItems + 1), INVENTORY_POWER_BRACELET);
    gb_write(&gb, hPressedButtonsMask, J_A | J_DOWN); /* pulling down when facing UP */
    gb_write(&gb, wActivePowerUp, ACTIVE_POWER_UP_PIECE_OF_POWER); /* threshold = 3 */
    gb_write(&gb, wPullCounter, 2); /* will increment to 3 -> triggers lift! */

    mock_reveal_called = false;
    mock_5795_called = false;

    label_1F69(&gb, mock_get_physics_flags, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
               mock_reveal_object, mock_spawn_proj, mock_func_003_5795);

    TEST_ASSERT(gb_read(&gb, hLinkInteractiveMotionBlocked) == 1, "Motion not blocked during lift");
    TEST_ASSERT(mock_reveal_called, "Reveal object not called on lift completion");
    TEST_ASSERT(mock_5795_called, "func_5795 not called on lift completion");
}

#define RUN_LINK_TEST(fn, name) \
    do { \
        int _prev = failures; \
        printf("[RUN ] %s\n", name); \
        fn(); \
        if (failures == _prev) { \
            printf("[PASS] %s\n", name); \
        } else { \
            printf("[FAIL] %s\n", name); \
        } \
    } while (0)

void run_link_tests(void) {
    printf("[TEST] Link\n");
    RUN_LINK_TEST(test_compute_link_position, "ComputeLinkPosition");
    RUN_LINK_TEST(test_func_21e1_z_velocity, "func_21E1_ZVelocity");
    RUN_LINK_TEST(test_label_2183_and_func_2165, "label_2183_and_func_2165");
    RUN_LINK_TEST(test_label_1f69_interactive_motion, "label_1F69_InteractiveMotion");
    RUN_LINK_TEST(test_disableMovement_and_playNoiseStairs, "disableMovementInTransition");
    RUN_LINK_TEST(test_fade_out_transitions, "MapFadeOutTransition");
    RUN_LINK_TEST(test_resets_and_position, "LinkResetAndPosition");
    RUN_LINK_TEST(test_update_link_walking_animation_trampoline, "UpdateLinkWalkingAnimation_trampoline");
    RUN_LINK_TEST(test_clear_link_position_increment, "ClearLinkPositionIncrement");
    RUN_LINK_TEST(test_use_pegasus_boots, "UsePegasusBoots");
    RUN_LINK_TEST(test_display_transient_vfx_for_link_running, "DisplayTransientVfxForLinkRunning");
    RUN_LINK_TEST(test_check_items_sword_collision, "CheckItemsSwordCollision");
    RUN_LINK_TEST(test_check_static_sword_collision, "CheckStaticSwordCollision");
    RUN_LINK_TEST(test_check_static_sword_collision_trampoline, "CheckStaticSwordCollision_trampoline");
    RUN_LINK_TEST(test_func_1819_and_1828, "func_1819_and_func_1828");
    RUN_LINK_TEST(test_func_1a22_and_1a39, "func_1A22_and_func_1A39");
    RUN_LINK_TEST(test_apply_link_motion_state, "ApplyLinkMotionState");
    RUN_LINK_TEST(test_set_spawn_location_and_label_19da, "SetSpawnLocation_and_label_19DA");
    RUN_LINK_TEST(test_link_motion_map_fade_in_handler, "LinkMotionMapFadeInHandler");
    RUN_LINK_TEST(test_link_motion_map_fade_out_handler, "LinkMotionMapFadeOutHandler");
    RUN_LINK_TEST(test_update_link_walking_animation, "UpdateLinkWalkingAnimation");

    if (failures == 0) {
        printf("[PASS] Link\n\n");
    } else {
        printf("[FAIL] Link (%d failures)\n\n", failures);
    }
}
