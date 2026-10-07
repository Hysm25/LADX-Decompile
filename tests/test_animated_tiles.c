#include "home/animated_tiles.h"
#include "constants/hardware.h"
#include "constants/memory.h"
#include "constants/gameplay.h"
#include "constants/link.h"
#include "constants/gfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++; \
        } \
    } while (0)

static int failures = 0;
static int mock_skip_calls = 0;
static int mock_counter_calls = 0;
static int mock_photo_calls = 0;
static int mock_config_calls = 0;

static uint8_t test_rom[0x4000 * 0x40]; /* 64 banks */

static void init_test_gb(GBState *gb) {
    gb_init(gb);
    memset(test_rom, 0, sizeof(test_rom));
    gb->rom = test_rom;
    gb->rom_size = sizeof(test_rom);
}

static void mock_skip_func(GBState *gb) {
    (void)gb;
    mock_skip_calls++;
}

static void mock_counter_func(GBState *gb) {
    (void)gb;
    mock_counter_calls++;
}

static void mock_photo_func(GBState *gb) {
    (void)gb;
    mock_photo_calls++;
}

static void mock_config_func(GBState *gb, uint8_t flag, uint8_t *out_bank) {
    (void)gb;
    mock_config_calls++;
    if (flag == 1) {
        *out_bank = 0x22;
    } else {
        *out_bank = 0x23;
    }
}

static void test_increment_animated_tiles_data_offset(void) {
    GBState gb;
    init_test_gb(&gb);
    gb_write(&gb, hAnimatedTilesDataOffset, 0x40);

    uint8_t res = IncrementAnimatedTilesDataOffset(&gb);
    TEST_ASSERT(res == 0x80, "IncrementAnimatedTilesDataOffset did not add 0x40");
    TEST_ASSERT(gb_read(&gb, hAnimatedTilesDataOffset) == 0x80, "hAnimatedTilesDataOffset register mismatch");

    res = IncrementAnimatedTilesDataOffset(&gb);
    TEST_ASSERT(res == 0xC0, "Second increment failed");
}

static void test_skip_tiles_group_animation(void) {
    GBState gb;
    init_test_gb(&gb);
    mock_skip_calls = 0;

    SkipTilesGroupAnimation(&gb, mock_skip_func);
    TEST_ASSERT(mock_skip_calls == 1, "func_020_54f5 not called");
    TEST_ASSERT(gb.rom_bank == 0x0C, "ROM bank not adjusted to 0x0C");
}

static void test_load_animated_tiles_frame(void) {
    GBState gb;
    init_test_gb(&gb);
    gb.rom_bank = 0x0C;

    /* Setup 0x40 source bytes at bank 0x0C offset (0x6A00 - 0x4000) */
    size_t rom_offset = (size_t)0x0C * 0x4000 + (0x6A00 - 0x4000);
    for (int i = 0; i < 0x40; i++) {
        test_rom[rom_offset + i] = (uint8_t)(i + 1);
    }

    LoadAnimatedTilesFrame(&gb, 0x6A00, 0x96C0, NULL);
    for (int i = 0; i < 0x40; i++) {
        TEST_ASSERT(gb_read(&gb, 0x96C0 + i) == (uint8_t)(i + 1), "VRAM copy mismatch in LoadAnimatedTilesFrame");
    }

    /* Test MAP_COLOR_DUNGEON special branch */
    gb_write(&gb, hMapId, MAP_COLOR_DUNGEON);
    mock_config_calls = 0;
    LoadAnimatedTilesFrame(&gb, 0x6A00, 0x96C0, mock_config_func);
    TEST_ASSERT(mock_config_calls == 2, "ConfigureAnimatedTilesCopy not called twice for Color Dungeon");
}

static void test_animate_counter_tiles_group(void) {
    GBState gb;

    /* Frame not matching (count & 7 != 0) -> calls Skip */
    init_test_gb(&gb);
    gb_write(&gb, hAnimatedTilesFrameCount, 1);
    mock_skip_calls = 0;
    mock_counter_calls = 0;
    AnimateCounterTilesGroup(&gb, mock_counter_func, mock_skip_func);
    TEST_ASSERT(mock_skip_calls == 1, "Skip not called on non-matching frame");
    TEST_ASSERT(mock_counter_calls == 0, "Counter called on non-matching frame");

    /* Frame matching (count & 7 == 0) -> calls counter routine */
    gb_write(&gb, hAnimatedTilesFrameCount, 8);
    mock_skip_calls = 0;
    mock_counter_calls = 0;
    AnimateCounterTilesGroup(&gb, mock_counter_func, mock_skip_func);
    TEST_ASSERT(mock_counter_calls == 1, "Counter routine not called on matching frame");
    TEST_ASSERT(gb.rom_bank == 0x0C, "ROM bank not set to 0x0C after counter tiles update");
}

static void test_slow_speed_tile_groups(void) {
    GBState gb;
    init_test_gb(&gb);

    /* Test slow speed skip (frame & 0xF != 0) */
    gb_write(&gb, hAnimatedTilesFrameCount, 3);
    mock_skip_calls = 0;
    AnimateTideTilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(mock_skip_calls == 1, "Tide group skip failed");

    AnimateVillageTilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(mock_skip_calls == 2, "Village group skip failed");

    AnimateWaterDungeonTilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(mock_skip_calls == 3, "Water dungeon group skip failed");

    AnimateSlowWaterfallTilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(mock_skip_calls == 4, "Slow waterfall group skip failed");

    AnimateUndergroundTilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(mock_skip_calls == 5, "Underground group skip failed");

    /* Test slow speed frame match (frame & 0xF == 0) */
    gb_write(&gb, hAnimatedTilesFrameCount, 0x10);
    gb_write(&gb, hAnimatedTilesDataOffset, 0);
    AnimateTideTilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(gb_read(&gb, hAnimatedTilesDataOffset) == 0x40, "Offset not incremented by AnimateTideTilesGroup");
}

static void test_medium_and_fast_speed_tile_groups(void) {
    GBState gb;
    init_test_gb(&gb);

    /* Medium speed skips on frame & 7 != 0 */
    gb_write(&gb, hAnimatedTilesFrameCount, 1);
    mock_skip_calls = 0;
    AnimateWarpTilesGroup(&gb, mock_skip_func, NULL);
    AnimateBubblesTilesGroup(&gb, mock_skip_func, NULL);
    AnimateWeatherVaneTilesGroup(&gb, mock_skip_func, NULL);
    AnimateCrystalBlockTilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(mock_skip_calls == 4, "Medium speed groups skip failed");

    /* Fast speed skips on frame & 3 != 0 */
    gb_write(&gb, hAnimatedTilesFrameCount, 1);
    mock_skip_calls = 0;
    AnimateWaterCurrentsTilesGroup(&gb, mock_skip_func, NULL);
    AnimateWaterfallTilesGroup(&gb, mock_skip_func, NULL);
    AnimateLightBeamTilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(mock_skip_calls == 3, "Fast speed groups skip failed");

    /* Fast speed executes on frame & 3 == 0 */
    gb_write(&gb, hAnimatedTilesFrameCount, 4);
    gb_write(&gb, hAnimatedTilesDataOffset, 0);
    AnimateWaterCurrentsTilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(gb_read(&gb, hAnimatedTilesDataOffset) == 0x40, "Offset not incremented by fast speed group");
}

static void test_dungeon1_and_lava_tiles_groups(void) {
    GBState gb;
    init_test_gb(&gb);
    gb.rom_bank = 0x0C;

    /* Frame not matching */
    gb_write(&gb, hAnimatedTilesFrameCount, 2);
    mock_skip_calls = 0;
    AnimateDungeon1TilesGroup(&gb, mock_skip_func, NULL);
    AnimateLavaTilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(mock_skip_calls == 2, "Dungeon 1 / Lava skip failed");

    /* Frame matching (0x10 -> index 2 -> offset 0x80) */
    gb_write(&gb, hAnimatedTilesFrameCount, 0x10);
    size_t rom_offset = (size_t)0x0C * 0x4000 + (0x6D80 - 0x4000);
    for (int i = 0; i < 0x40; i++) {
        test_rom[rom_offset + i] = 0xAA;
    }
    AnimateDungeon1TilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(gb_read(&gb, 0x96C0) == 0xAA, "Dungeon 1 tile copy mismatch");
}

static void test_dungeon2_and_label_1cb8(void) {
    GBState gb;
    init_test_gb(&gb);

    /* Non-Color Dungeon: calls label_1CB8 */
    gb_write(&gb, hMapId, 0x01);
    /* next_count = 1 + 1 = 2 -> (2 & 3) != 0 -> AnimateDungeon1TilesGroup */
    gb_write(&gb, hAnimatedTilesFrameCount, 1);
    mock_skip_calls = 0;
    AnimateDungeon2TilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(mock_skip_calls == 1, "Dungeon 2 forward to Dungeon 1 failed");

    /* Color Dungeon: loads from wAnimatedScrollingTilesStorage to 0x8400 */
    gb_write(&gb, hMapId, MAP_COLOR_DUNGEON);
    for (int i = 0; i < 0x40; i++) {
        gb_write(&gb, wAnimatedScrollingTilesStorage + i, 0x55);
    }
    AnimateDungeon2TilesGroup(&gb, mock_skip_func, NULL);
    TEST_ASSERT(gb_read(&gb, 0x8400) == 0x55, "Color Dungeon 2 scrolling tiles copy mismatch");
}

static void test_animate_photo_tiles_group(void) {
    GBState gb;
    init_test_gb(&gb);
    mock_photo_calls = 0;

    AnimatePhotoTilesGroup(&gb, mock_photo_func);
    TEST_ASSERT(mock_photo_calls == 1, "Photo group callback not called");
    TEST_ASSERT(gb.rom_bank == 0x38, "ROM bank not set to 0x38 for photo group");
}

static void test_copy_link_tiles_pair(void) {
    GBState gb;
    init_test_gb(&gb);

    /* Use WRAM source */
    for (int i = 0; i < 8; i++) {
        gb_write(&gb, 0xC500 + i, (uint8_t)(i + 0x10));
    }

    CopyLinkTilesPair(&gb, 0xC500, 0x8000, 8);
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT(gb_read(&gb, 0x8000 + i) == (uint8_t)(i + 0x10), "CopyLinkTilesPair data mismatch");
    }
    TEST_ASSERT(gb.rom_bank == 0x20, "ROM bank not restored to 0x20 after CopyLinkTilesPair");
}

static void test_animate_marin_beach_tiles(void) {
    GBState gb;

    /* 1. Pending draw command -> returns early */
    init_test_gb(&gb);
    gb_write(&gb, wDrawCommand, 1);
    AnimateMarinBeachTiles(&gb);
    TEST_ASSERT(gb.rom_bank == 1, "ROM bank changed despite draw command active");

    /* 2. Valid frame counter & 0x0F == 0 */
    init_test_gb(&gb);
    gb_write(&gb, wDrawCommand, 0);
    gb_write(&gb, hFrameCounter, 0x00); /* & 0x0F == 0, & 0x30 == 0 */
    size_t rom_offset = (size_t)0x10 * 0x4000 + (0x6500 - 0x4000);
    for (int i = 0; i < 0x40; i++) {
        test_rom[rom_offset + i] = (uint8_t)(i + 0x77);
    }
    AnimateMarinBeachTiles(&gb);
    TEST_ASSERT(gb_read(&gb, 0x9500) == 0x77, "Marin beach tiles copy mismatch");
    TEST_ASSERT(gb.rom_bank == 0x10, "ROM bank not set to 0x10");
}


static void test_draw_link_sprite(void) {
    GBState gb;
    init_test_gb(&gb);

    /* 1. Animation state 0xFF: early return */
    gb_write(&gb, hLinkAnimationState, 0xFF);
    gb_write(&gb, (uint16_t)(wLinkOAMBuffer + 8), 0xAA);
    DrawLinkSprite(&gb);
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 8)) == 0xAA, "DrawLinkSprite did not return on 0xFF state");

    /* 2. Standard DMG Link sprite */
    gb_write(&gb, hLinkAnimationState, 0x01);
    gb_write(&gb, hIsGBC, 0x00);
    gb_write(&gb, wInvincibilityCounter, 0x04);
    gb_write(&gb, wC13B, 0x20);
    gb_write(&gb, wC145, 0x10); /* Y = 0x30 */
    gb_write(&gb, hLinkPositionX, 0x40);
    gb_write(&gb, wC13C, 0x02); /* X1 = 0x42, X2 = 0x4A */
    gb_write(&gb, wC11D, 0x00);
    gb_write(&gb, wC11E, 0x20);

    DrawLinkSprite(&gb);

    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 8)) == 0x30, "Sprite 0 Y mismatch");
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 9)) == 0x42, "Sprite 0 X mismatch");
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 10)) == 0x00, "Sprite 0 Tile mismatch");
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 11)) == 0x10, "Sprite 0 Flags mismatch");

    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 12)) == 0x30, "Sprite 1 Y mismatch");
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 13)) == 0x4A, "Sprite 1 X mismatch");
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 14)) == 0x02, "Sprite 1 Tile mismatch");
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 15)) == 0x30, "Sprite 1 Flags mismatch");

    /* 3. GBC Tunic and Swimming modifier */
    gb_write(&gb, hIsGBC, 0x01);
    gb_write(&gb, wInvincibilityCounter, 0x00);
    gb_write(&gb, wTunicType, 0x01); /* Blue tunic -> tunic + 1 = 2 */
    DrawLinkSprite(&gb);
    TEST_ASSERT((gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 11)) & 0x03) == 0x02, "Tunic palette mismatch");

    /* Swimming */
    gb_write(&gb, hLinkAnimationState, LINK_ANIMATION_STATE_HOLD_SWIMMING_2);
    DrawLinkSprite(&gb);
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 11)) == 0x03, "Swimming sprite 0 flags mismatch");
    TEST_ASSERT(gb_read(&gb, (uint16_t)(wLinkOAMBuffer + 15)) == 0x23, "Swimming sprite 1 flags mismatch");
}

static void test_replace_marin_tiles(void) {
    GBState gb;
    init_test_gb(&gb);

    /* Sitting = false -> from 0x4F00 to 0x8400 */
    size_t rom_offset = (size_t)0x0E * 0x4000 + (0x4F00 - 0x4000);
    test_rom[rom_offset] = 0x99;
    ReplaceMarinTiles(&gb, false);
    TEST_ASSERT(gb_read(&gb, 0x8400) == 0x99, "Marin standing tile mismatch");
    TEST_ASSERT(gb_read(&gb, hReplaceTiles) == 0, "hReplaceTiles not cleared");

    /* Sitting = true -> from 0x6080 to 0x8400 */
    rom_offset = (size_t)0x12 * 0x4000 + (0x6080 - 0x4000);
    test_rom[rom_offset] = 0x88;
    ReplaceMarinTiles(&gb, true);
    TEST_ASSERT(gb_read(&gb, 0x8400) == 0x88, "Marin sitting tile mismatch");
}

static void test_replace_trading_item_tiles(void) {
    GBState gb;
    init_test_gb(&gb);

    /* Item < 2 -> early exit */
    gb_write(&gb, wTradeSequenceItem, 0x01);
    ReplaceTradingItemTiles(&gb);
    TEST_ASSERT(gb_read(&gb, hReplaceTiles) == 0, "hReplaceTiles not cleared");

    /* Item = 3 -> de = 1 * 0x40 = 0x40 -> src = 0x4440 */
    gb_write(&gb, wTradeSequenceItem, 0x03);
    size_t rom_offset = (size_t)0x0C * 0x4000 + (0x4440 - 0x4000);
    test_rom[rom_offset] = 0x77;
    ReplaceTradingItemTiles(&gb);
    TEST_ASSERT(gb_read(&gb, 0x89A0) == 0x77, "Trading item tile mismatch");
}

static void test_replace_tile_pairs_and_buttons(void) {
    GBState gb;
    init_test_gb(&gb);

    /* ReplaceTilesButtonPressed -> 0x7F00 to 0x9140 */
    size_t rom_offset = (size_t)0x12 * 0x4000 + (0x7F00 - 0x4000);
    test_rom[rom_offset] = 0x55;
    ReplaceTilesButtonPressed(&gb);
    TEST_ASSERT(gb_read(&gb, 0x9140) == 0x55, "Button pressed tile mismatch");

    /* ReplaceTiles_04 -> 0x4CC0 to 0x9140 */
    rom_offset = (size_t)0x0D * 0x4000 + (0x4CC0 - 0x4000);
    test_rom[rom_offset] = 0x66;
    ReplaceTiles_04(&gb);
    TEST_ASSERT(gb_read(&gb, 0x9140) == 0x66, "ReplaceTiles_04 mismatch");

    /* Magic powder by toadstool */
    rom_offset = (size_t)0x0C * 0x4000 + (0x68C0 - 0x4000);
    test_rom[rom_offset] = 0x33;
    ReplaceMagicPowderTilesByToadstool(&gb);
    TEST_ASSERT(gb_read(&gb, 0x88E0) == 0x33, "Magic powder by toadstool mismatch");

    /* Slime key by golden leaf */
    rom_offset = (size_t)0x0C * 0x4000 + (0x68E0 - 0x4000);
    test_rom[rom_offset] = 0x44;
    ReplaceSlimeKeyTilesByGoldenLeaf(&gb);
    TEST_ASSERT(gb_read(&gb, 0x8CA0) == 0x44, "Slime key by golden leaf mismatch");

    /* Toadstool by magic powder */
    rom_offset = (size_t)0x0C * 0x4000 + (0x48E0 - 0x4000);
    test_rom[rom_offset] = 0x22;
    ReplaceToadstoolTilesByMagicPowder(&gb);
    TEST_ASSERT(gb_read(&gb, 0x88E0) == 0x22, "Toadstool by magic powder mismatch");
}

static void test_replace_credits_and_instruments(void) {
    GBState gb;
    init_test_gb(&gb);

    /* ReplaceDialogTilesByInstruments */
    gb_write(&gb, wCreditsScratch0, 0x01); /* offset = 0x40 */
    size_t rom_offset = (size_t)0x11 * 0x4000 + (0x5040 - 0x4000);
    test_rom[rom_offset] = 0x12;
    ReplaceDialogTilesByInstruments(&gb);
    TEST_ASSERT(gb_read(&gb, 0x8D40) == 0x12, "Dialog instrument tile mismatch");

    /* ReplaceTiles_08 */
    gb_write(&gb, wCreditsScratch0, 0x02); /* offset = 0x80 */
    rom_offset = (size_t)0x13 * 0x4000 + (0x4D80 - 0x4000);
    test_rom[rom_offset] = 0x34;
    ReplaceTiles_08(&gb);
    TEST_ASSERT(gb_read(&gb, 0x8D80) == 0x34, "ReplaceTiles_08 mismatch");
}

static void test_update_switch_block_tiles(void) {
    GBState gb;
    init_test_gb(&gb);

    /* Stage 2 -> stage 3: toggles wSwitchBlocksState */
    gb_write(&gb, wSwitchBlocksState, 0);
    UpdateSwitchBlockTiles(&gb, 2);
    TEST_ASSERT(gb_read(&gb, wSwitchBlocksState) == 2, "wSwitchBlocksState not toggled at stage 3");
    TEST_ASSERT(gb_read(&gb, wSwitchableObjectAnimationStage) == 3, "Animation stage mismatch");
    TEST_ASSERT(gb_read(&gb, hLinkInteractiveMotionBlocked) == 1, "hLinkInteractiveMotionBlocked not set");

    /* Stage 9 -> stage 10: clears stage */
    UpdateSwitchBlockTiles(&gb, 9);
    TEST_ASSERT(gb_read(&gb, wSwitchableObjectAnimationStage) == 0, "Stage not cleared at stage 10");
}

static void test_animate_tiles_dispatcher(void) {
    GBState gb;
    init_test_gb(&gb);

    /* 1. WindowY != 0x80 -> ignored */
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, wWindowY, 0x00);
    gb_write(&gb, hAnimatedTilesFrameCount, 0x05);
    AnimateTiles(&gb, NULL, NULL, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, hAnimatedTilesFrameCount) == 0x05, "AnimateTiles ran despite window active");

    /* 2. WindowY == 0x80 -> advances frame count */
    gb_write(&gb, wWindowY, 0x80);
    gb_write(&gb, wInventoryAppearing, 0);
    gb_write(&gb, wRoomTransitionState, 0);
    gb_write(&gb, wDrawCommand, 0);
    gb_write(&gb, wSwitchableObjectAnimationStage, 0);
    gb_write(&gb, hReplaceTiles, 0);
    gb_write(&gb, hAnimatedTilesGroup, ANIMATED_TILES_TIDE);
    AnimateTiles(&gb, NULL, mock_skip_func, NULL, NULL);
    TEST_ASSERT(gb_read(&gb, hAnimatedTilesFrameCount) == 0x06, "hAnimatedTilesFrameCount not incremented");
}


static void test_replace_evil_eagle_rider_tiles(void) {
    GBState gb;
    init_test_gb(&gb);

    /* Visible tiles (00:3F93) */
    size_t rom_vis_1 = (size_t)BANK_EvilEagleRiderVisibleTiles * 0x4000 + (EvilEagleRiderVisibleTiles - 0x4000);
    size_t rom_vis_2 = rom_vis_1 + 0x10;
    test_rom[rom_vis_1] = 0x55;
    test_rom[rom_vis_2] = 0x66;

    gb_write(&gb, hReplaceTiles, REPLACE_TILES_UNKNOWN_04);
    ReplaceEvilEagleRiderVisibleTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x460) == 0x55, "Evil eagle visible tile 1 mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x480) == 0x66, "Evil eagle visible tile 2 mismatch");
    TEST_ASSERT(gb_read(&gb, hReplaceTiles) == 0, "hReplaceTiles not cleared");
    TEST_ASSERT(gb.rom_bank == BANK_LinkCharacterTiles, "Bank not restored to LinkCharacterTiles");

    /* Hidden tiles (00:3FA9) */
    size_t rom_hid_1 = (size_t)BANK_EvilEagleRiderHiddenTiles * 0x4000 + (EvilEagleRiderHiddenTiles - 0x4000);
    size_t rom_hid_2 = rom_hid_1 + 0x10;
    test_rom[rom_hid_1] = 0x77;
    test_rom[rom_hid_2] = 0x88;

    gb_write(&gb, hReplaceTiles, REPLACE_TILES_UNKNOWN_08);
    ReplaceEvilEagleRiderHiddenTiles(&gb);
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x460) == 0x77, "Evil eagle hidden tile 1 mismatch");
    TEST_ASSERT(gb_read(&gb, vTiles0 + 0x480) == 0x88, "Evil eagle hidden tile 2 mismatch");
    TEST_ASSERT(gb_read(&gb, hReplaceTiles) == 0, "hReplaceTiles not cleared");
}

#define RUN_ANIM_TEST(fn, name) \
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

void run_animated_tiles_tests(void) {
    printf("[TEST] Animated Tiles\n");
    RUN_ANIM_TEST(test_replace_evil_eagle_rider_tiles, "ReplaceEvilEagleRiderTiles");
    RUN_ANIM_TEST(test_draw_link_sprite, "DrawLinkSprite");
    RUN_ANIM_TEST(test_replace_marin_tiles, "ReplaceMarinTiles");
    RUN_ANIM_TEST(test_replace_trading_item_tiles, "ReplaceTradingItemTiles");
    RUN_ANIM_TEST(test_replace_tile_pairs_and_buttons, "ReplaceTilePairsAndButtons");
    RUN_ANIM_TEST(test_replace_credits_and_instruments, "ReplaceCreditsAndInstruments");
    RUN_ANIM_TEST(test_update_switch_block_tiles, "UpdateSwitchBlockTiles");
    RUN_ANIM_TEST(test_animate_tiles_dispatcher, "AnimateTilesDispatcher");
    RUN_ANIM_TEST(test_increment_animated_tiles_data_offset, "IncrementAnimatedTilesDataOffset");
    RUN_ANIM_TEST(test_skip_tiles_group_animation, "SkipTilesGroupAnimation");
    RUN_ANIM_TEST(test_load_animated_tiles_frame, "LoadAnimatedTilesFrame");
    RUN_ANIM_TEST(test_animate_counter_tiles_group, "AnimateCounterTilesGroup");
    RUN_ANIM_TEST(test_slow_speed_tile_groups, "SlowSpeedAnimatedTileGroups");
    RUN_ANIM_TEST(test_medium_and_fast_speed_tile_groups, "MediumAndFastSpeedAnimatedTileGroups");
    RUN_ANIM_TEST(test_dungeon1_and_lava_tiles_groups, "Dungeon1AndLavaTileGroups");
    RUN_ANIM_TEST(test_dungeon2_and_label_1cb8, "Dungeon2AndLabel1CB8");
    RUN_ANIM_TEST(test_animate_photo_tiles_group, "AnimatePhotoTilesGroup");
    RUN_ANIM_TEST(test_copy_link_tiles_pair, "CopyLinkTilesPair");
    RUN_ANIM_TEST(test_animate_marin_beach_tiles, "AnimateMarinBeachTiles");

    if (failures == 0) {
        printf("[PASS] Animated Tiles\n\n");
    } else {
        printf("[FAIL] Animated Tiles (%d failures)\n\n", failures);
        exit(1);
    }
}
