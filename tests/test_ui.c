#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "gb.h"
#include "home/ui.h"
#include "constants/hardware.h"
#include "constants/memory.h"
#include "constants/gfx.h"

static int failures = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++; \
        } \
    } while (0)

static void test_copy_tiles_to_piece_of_heart_meter(void) {
    GBState gb;
    gb_init(&gb);

    /* Allocate simulated ROM in bank $0C */
    uint8_t test_rom[0x34000];
    memset(test_rom, 0, sizeof(test_rom));

    /* Populate bank $0C source at $6900 */
    uint32_t rom_offset = (0x0C * 0x4000) + (0x6900 - 0x4000);
    for (int i = 0; i < 0x30; i++) {
        test_rom[rom_offset + i] = (uint8_t)(0xA0 + i);
    }

    gb_attach_rom(&gb, test_rom, sizeof(test_rom));
    gb.rom_bank = 0x0C;

    gb_write(&gb, hNeedsUpdatingBGTiles, 5);
    gb_write(&gb, hBGTilesLoadingStage, 2);

    CopyTilesToPieceOfHeartMeter(&gb, 0x89A0, 0x6900);

    /* Verify 0x30 bytes copied to VRAM at 0x89A0 */
    for (int i = 0; i < 0x30; i++) {
        TEST_ASSERT(gb.vram[0][0x9A0 + i] == (uint8_t)(0xA0 + i), "Copied byte mismatch");
    }

    /* Verify flags and bank */
    TEST_ASSERT(gb_read(&gb, hNeedsUpdatingBGTiles) == 0, "hNeedsUpdatingBGTiles not cleared");
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 0, "hBGTilesLoadingStage not cleared");
    TEST_ASSERT(gb.rom_bank == 0x0C, "ROM bank not restored to 0x0C");
}

static void test_load_piece_of_heart_meter_tiles(void) {
    GBState gb;
    gb_init(&gb);

    uint8_t test_rom[0x34000];
    memset(test_rom, 0, sizeof(test_rom));

    /* Setup PieceOfHeartMeterTiles at 0x6900 in bank $0C */
    uint32_t offset1 = (0x0C * 0x4000) + (PieceOfHeartMeterTiles - 0x4000);
    for (int i = 0; i < 0x30; i++) {
        test_rom[offset1 + i] = (uint8_t)(0x10 + i);
    }
    /* Setup PieceOfHeartMeterTiles + 0x30 at 0x6930 */
    uint32_t offset2 = offset1 + 0x30;
    for (int i = 0; i < 0x30; i++) {
        test_rom[offset2 + i] = (uint8_t)(0x40 + i);
    }

    gb_attach_rom(&gb, test_rom, sizeof(test_rom));
    gb.rom_bank = 0x0C;

    /* Test LoadPieceOfHeartMeterTiles1: dest = vTiles1 + $1A0 (0x89A0) */
    gb_write(&gb, hNeedsUpdatingBGTiles, 1);
    gb_write(&gb, hBGTilesLoadingStage, 1);
    LoadPieceOfHeartMeterTiles1(&gb);

    for (int i = 0; i < 0x30; i++) {
        TEST_ASSERT(gb.vram[0][0x9A0 + i] == (uint8_t)(0x10 + i), "LoadPieceOfHeartMeterTiles1 byte mismatch");
    }
    TEST_ASSERT(gb_read(&gb, hNeedsUpdatingBGTiles) == 0, "Flags not cleared after Load 1");
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 0, "Stage not cleared after Load 1");
    TEST_ASSERT(gb.rom_bank == 0x0C, "Bank not 0x0C after Load 1");

    /* Test LoadPieceOfHeartMeterTiles2: dest = vTiles1 + $1D0 (0x89D0) */
    gb_write(&gb, hNeedsUpdatingBGTiles, 1);
    gb_write(&gb, hBGTilesLoadingStage, 1);
    LoadPieceOfHeartMeterTiles2(&gb);

    for (int i = 0; i < 0x30; i++) {
        TEST_ASSERT(gb.vram[0][0x9D0 + i] == (uint8_t)(0x40 + i), "LoadPieceOfHeartMeterTiles2 byte mismatch");
    }
    TEST_ASSERT(gb_read(&gb, hNeedsUpdatingBGTiles) == 0, "Flags not cleared after Load 2");
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 0, "Stage not cleared after Load 2");
    TEST_ASSERT(gb.rom_bank == 0x0C, "Bank not 0x0C after Load 2");
}

static void test_clear_piece_of_heart_meter_tiles(void) {
    GBState gb;
    gb_init(&gb);

    uint8_t test_rom[0x34000];
    memset(test_rom, 0, sizeof(test_rom));

    /* Setup InventoryEquipmentItemsTiles + $1D0 at 0x49D0 in bank $0C */
    uint32_t offset1 = (0x0C * 0x4000) + (InventoryEquipmentItemsTiles + 0x1D0 - 0x4000);
    for (int i = 0; i < 0x30; i++) {
        test_rom[offset1 + i] = (uint8_t)(0x70 + i);
    }
    /* Setup InventoryEquipmentItemsTiles + $1A0 at 0x49A0 */
    uint32_t offset2 = (0x0C * 0x4000) + (InventoryEquipmentItemsTiles + 0x1A0 - 0x4000);
    for (int i = 0; i < 0x30; i++) {
        test_rom[offset2 + i] = (uint8_t)(0x90 + i);
    }

    gb_attach_rom(&gb, test_rom, sizeof(test_rom));
    gb.rom_bank = 0x0C;

    /* Test ClearPieceOfHeartMeterTiles1: restores $49D0 to vTiles1 + $1D0 (0x89D0) */
    gb_write(&gb, hNeedsUpdatingBGTiles, 1);
    gb_write(&gb, hBGTilesLoadingStage, 1);
    ClearPieceOfHeartMeterTiles1(&gb);

    for (int i = 0; i < 0x30; i++) {
        TEST_ASSERT(gb.vram[0][0x9D0 + i] == (uint8_t)(0x70 + i), "ClearPieceOfHeartMeterTiles1 byte mismatch");
    }
    TEST_ASSERT(gb_read(&gb, hNeedsUpdatingBGTiles) == 0, "Flags not cleared after Clear 1");
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 0, "Stage not cleared after Clear 1");
    TEST_ASSERT(gb.rom_bank == 0x0C, "Bank not 0x0C after Clear 1");

    /* Test ClearPieceOfHeartMeterTiles2: restores $49A0 to vTiles1 + $1A0 (0x89A0) */
    gb_write(&gb, hNeedsUpdatingBGTiles, 1);
    gb_write(&gb, hBGTilesLoadingStage, 1);
    ClearPieceOfHeartMeterTiles2(&gb);

    for (int i = 0; i < 0x30; i++) {
        TEST_ASSERT(gb.vram[0][0x9A0 + i] == (uint8_t)(0x90 + i), "ClearPieceOfHeartMeterTiles2 byte mismatch");
    }
    TEST_ASSERT(gb_read(&gb, hNeedsUpdatingBGTiles) == 0, "Flags not cleared after Clear 2");
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 0, "Stage not cleared after Clear 2");
    TEST_ASSERT(gb.rom_bank == 0x0C, "Bank not 0x0C after Clear 2");
}

static int s8_called = 0, s9_called = 0, s10_called = 0, s11_called = 0;
static void hook_stage8(GBState *gb) { s8_called++; (void)gb; }
static void hook_stage9(GBState *gb) { s9_called++; (void)gb; }
static void hook_stage10(GBState *gb) { s10_called++; (void)gb; }
static void hook_stage11(GBState *gb) { s11_called++; (void)gb; }

static void test_load_dungeon_minimap_tiles(void) {
    GBState gb;
    gb_init(&gb);

    /* Allocate simulated ROM covering up to bank $33 (0xCC000 bytes) */
    static uint8_t test_rom[0xD0000];
    memset(test_rom, 0, sizeof(test_rom));

    /* Populate bank $12 (DMG) and bank $32 (GBC) at 0x7E00 */
    uint32_t dmg_offset = (0x12 * 0x4000) + (DungeonMinimapTiles - 0x4000);
    uint32_t gbc_offset = (0x32 * 0x4000) + (DungeonMinimapTiles - 0x4000);
    for (int i = 0; i < 0x200; i++) {
        test_rom[dmg_offset + i] = (uint8_t)(0x50 + (i & 0xFF));
        test_rom[gbc_offset + i] = (uint8_t)(0x80 + (i & 0xFF));
    }
    gb_attach_rom(&gb, test_rom, sizeof(test_rom));

    /* 1. Test stages 0 through 7 in DMG mode (hIsGBC = 0) */
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, hBGTilesLoadingStage, 0);

    for (int stage = 0; stage < 8; stage++) {
        TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == stage, "Pre-stage mismatch");
        LoadDungeonMinimapTiles(&gb);
        TEST_ASSERT(gb.rom_bank == 0x12, "DMG bank should be 0x12");
        TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == stage + 1, "Stage not incremented");

        /* Check that 0x40 bytes were copied to vTiles1 + $500 + offset (0x8D00 + offset) */
        uint16_t vram_offset = 0xD00 + (stage * 0x40);
        for (int i = 0; i < 0x40; i++) {
            TEST_ASSERT(gb.vram[0][vram_offset + i] == (uint8_t)(0x50 + ((stage * 0x40 + i) & 0xFF)),
                        "Minimap tile data mismatch in VRAM");
        }
    }

    /* 2. Test palette stages 8, 9, 10, 11 with hooks */
    s8_called = s9_called = s10_called = s11_called = 0;

    /* Stage 8 */
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 8, "Stage should be 8");
    LoadDungeonMinimapTilesWithHooks(&gb, hook_stage8, hook_stage9, hook_stage10, hook_stage11);
    TEST_ASSERT(s8_called == 1, "Stage 8 hook not called");
    TEST_ASSERT(gb.rom_bank == 0x02, "ROM bank not switched to 2 for stage 8");
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 9, "Stage should now be 9");

    /* Stage 9 */
    LoadDungeonMinimapTilesWithHooks(&gb, hook_stage8, hook_stage9, hook_stage10, hook_stage11);
    TEST_ASSERT(s9_called == 1, "Stage 9 hook not called");
    TEST_ASSERT(gb.rom_bank == 0x02, "ROM bank not switched to 2 for stage 9");
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 10, "Stage should now be 10");

    /* Stage 10 */
    LoadDungeonMinimapTilesWithHooks(&gb, hook_stage8, hook_stage9, hook_stage10, hook_stage11);
    TEST_ASSERT(s10_called == 1, "Stage 10 hook not called");
    TEST_ASSERT(gb.rom_bank == 0x02, "ROM bank not switched to 2 for stage 10");
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 11, "Stage should now be 11");

    /* Stage 11 (palette stage 4 & finish) */
    gb_write(&gb, hNeedsUpdatingBGTiles, 1);
    LoadDungeonMinimapTilesWithHooks(&gb, hook_stage8, hook_stage9, hook_stage10, hook_stage11);
    TEST_ASSERT(s11_called == 1, "Stage 11 hook not called");
    TEST_ASSERT(gb.rom_bank == 0x02, "ROM bank not switched to 2 for stage 11");
    TEST_ASSERT(gb_read(&gb, hNeedsUpdatingBGTiles) == 0, "hNeedsUpdatingBGTiles not cleared");
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 0, "hBGTilesLoadingStage not reset to 0");

    /* 3. Test GBC mode tile copy (hIsGBC = 1 -> bank 0x32) */
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, hBGTilesLoadingStage, 3);
    LoadDungeonMinimapTiles(&gb);
    TEST_ASSERT(gb.rom_bank == 0x32, "GBC bank should be adjusted to 0x32");
    TEST_ASSERT(gb_read(&gb, hBGTilesLoadingStage) == 4, "Stage not incremented in GBC mode");
    uint16_t vram_offset = 0xD00 + (3 * 0x40);
    for (int i = 0; i < 0x40; i++) {
        TEST_ASSERT(gb.vram[0][vram_offset + i] == (uint8_t)(0x80 + ((3 * 0x40 + i) & 0xFF)),
                    "GBC minimap tile data mismatch");
    }
}

#define RUN_UI_TEST(name, code) \
    do { \
        int _prev = failures; \
        printf("[RUN ] %s\n", name); \
        code; \
        if (failures == _prev) { \
            printf("[PASS] %s\n", name); \
        } else { \
            printf("[FAIL] %s\n", name); \
        } \
    } while (0)

void run_ui_tests(void) {
    printf("[TEST] UI\n");
    RUN_UI_TEST("PieceOfHeartMeter", {
        test_copy_tiles_to_piece_of_heart_meter();
        test_load_piece_of_heart_meter_tiles();
        test_clear_piece_of_heart_meter_tiles();
    });
    RUN_UI_TEST("LoadDungeonMinimapTiles", test_load_dungeon_minimap_tiles());

    if (failures == 0) {
        printf("[PASS] UI\n\n");
    } else {
        printf("[FAIL] UI (%d failures)\n\n", failures);
    }
}
