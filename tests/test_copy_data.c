#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "gb.h"
#include "home/copy_data.h"
#include "constants/hardware.h"
#include "constants/memory.h"
#include "constants/gfx.h"
#include "constants/gameplay.h"

static int failures = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++; \
        } \
    } while (0)

static void test_copy_data_basic(void) {
    GBState gb;
    gb_init(&gb);

    /* Write test pattern to source (0xC000) */
    for (int i = 0; i < 16; i++) {
        gb_write(&gb, 0xC000 + i, (uint8_t)(i + 1));
    }

    /* Copy 16 bytes to destination (0xC100) */
    CopyData(&gb, 0xC100, 0xC000, 16);

    for (int i = 0; i < 16; i++) {
        TEST_ASSERT(gb_read(&gb, 0xC100 + i) == (uint8_t)(i + 1), "CopyData byte mismatch");
    }
}

static void test_copy_data_from_bank(void) {
    GBState gb;
    gb_init(&gb);

    /* Simulate a ROM buffer with 4 banks */
    uint8_t dummy_rom[0x10000];
    for (int i = 0; i < 0x10000; i++) {
        dummy_rom[i] = 0;
    }
    /* Bank 2 is at offset 0x8000 in ROM */
    dummy_rom[0x8000] = 0xAA;
    dummy_rom[0x8001] = 0xBB;
    dummy_rom[0x8002] = 0xCC;

    gb_attach_rom(&gb, dummy_rom, sizeof(dummy_rom));

    /* Read from bank 2 address 0x4000 (which maps to bank 2 offset 0x8000) */
    CopyDataFromBank(&gb, 2, 0xC000, 0x4000, 3);

    TEST_ASSERT(gb_read(&gb, 0xC000) == 0xAA, "CopyDataFromBank byte 0 incorrect");
    TEST_ASSERT(gb_read(&gb, 0xC001) == 0xBB, "CopyDataFromBank byte 1 incorrect");
    TEST_ASSERT(gb_read(&gb, 0xC002) == 0xCC, "CopyDataFromBank byte 2 incorrect");
    /* Verify bank was restored to 1 */
    TEST_ASSERT(gb.rom_bank == 1, "ROM bank was not restored to 1");
}

static void test_copy_data_trampoline(void) {
    GBState gb;
    gb_init(&gb);

    uint8_t rom[0x10000];
    memset(rom, 0, sizeof(rom));
    rom[3 * 0x4000 + 0] = 0x12;
    rom[3 * 0x4000 + 1] = 0x34;
    gb_attach_rom(&gb, rom, sizeof(rom));

    CopyData_trampoline(&gb, 3, 0xC100, 0x4000, 2);

    TEST_ASSERT(gb_read(&gb, 0xC100) == 0x12, "Trampoline byte 0 incorrect");
    TEST_ASSERT(gb_read(&gb, 0xC101) == 0x34, "Trampoline byte 1 incorrect");
    TEST_ASSERT(gb.rom_bank == 0x28, "ROM bank not restored to 0x28");
}

static void test_draw_command_copy_row_wrapping(void) {
    GBState gb;
    gb_init(&gb);

    /* Source payload at 0xC100 */
    uint16_t src = 0xC100;
    gb_write(&gb, 0xC100, 0x11);
    gb_write(&gb, 0xC101, 0x22);
    gb_write(&gb, 0xC102, 0x33);

    /* Destination at end of first row in vBGMap0: 0x981E */
    uint16_t dst = 0x981E;

    /* Command: DC_COPY_ROW (0x00) | (3 - 1) = 0x02 */
    uint8_t cmd = DC_COPY_ROW | 0x02;

    DrawCommandToVRAM(&gb, &src, dst, cmd);

    /* Byte 0 at 0x981E */
    TEST_ASSERT(gb_read(&gb, 0x981E) == 0x11, "Copy row byte 0 incorrect");
    /* Byte 1 at 0x981F */
    TEST_ASSERT(gb_read(&gb, 0x981F) == 0x22, "Copy row byte 1 incorrect");
    /* Byte 2: hl wrapped from 0x9820 back by 32 to 0x9800! */
    TEST_ASSERT(gb_read(&gb, 0x9800) == 0x33, "Copy row wrapping failed - expected at 0x9800");
    /* Source pointer should have advanced by 3 */
    TEST_ASSERT(src == 0xC103, "Source pointer incorrect after copy row");
}

static void test_draw_command_fill_row(void) {
    GBState gb;
    gb_init(&gb);

    uint16_t src = 0xC100;
    gb_write(&gb, 0xC100, 0x77);

    uint16_t dst = 0x9805;
    /* Command: DC_FILL_ROW (0x40) | (4 - 1) = 0x43 */
    uint8_t cmd = DC_FILL_ROW | 0x03;

    DrawCommandToVRAM(&gb, &src, dst, cmd);

    for (int i = 0; i < 4; i++) {
        TEST_ASSERT(gb_read(&gb, 0x9805 + i) == 0x77, "Fill row byte incorrect");
    }
    TEST_ASSERT(src == 0xC101, "Source pointer incorrect after fill row");
}

static void test_draw_command_copy_column(void) {
    GBState gb;
    gb_init(&gb);

    uint16_t src = 0xC100;
    gb_write(&gb, 0xC100, 0x01);
    gb_write(&gb, 0xC101, 0x02);
    gb_write(&gb, 0xC102, 0x03);

    uint16_t dst = 0x9802;
    /* Command: DC_COPY_COLUMN (0x80) | (3 - 1) = 0x82 */
    uint8_t cmd = DC_COPY_COLUMN | 0x02;

    DrawCommandToVRAM(&gb, &src, dst, cmd);

    TEST_ASSERT(gb_read(&gb, 0x9802) == 0x01, "Column byte 0 incorrect");
    TEST_ASSERT(gb_read(&gb, 0x9802 + 0x20) == 0x02, "Column byte 1 incorrect");
    TEST_ASSERT(gb_read(&gb, 0x9802 + 0x40) == 0x03, "Column byte 2 incorrect");
    TEST_ASSERT(src == 0xC103, "Source pointer incorrect after copy column");
}

static void test_draw_command_fill_column(void) {
    GBState gb;
    gb_init(&gb);

    uint16_t src = 0xC100;
    gb_write(&gb, 0xC100, 0x33);

    uint16_t dst = 0x980A;
    /* Command: DC_FILL_COLUMN (0xC0) | (3 - 1) = 0xC2 */
    uint8_t cmd = DC_FILL_COLUMN | 0x02;

    DrawCommandToVRAM(&gb, &src, dst, cmd);

    TEST_ASSERT(gb_read(&gb, 0x980A) == 0x33, "Fill column byte 0 incorrect");
    TEST_ASSERT(gb_read(&gb, 0x980A + 0x20) == 0x33, "Fill column byte 1 incorrect");
    TEST_ASSERT(gb_read(&gb, 0x980A + 0x40) == 0x33, "Fill column byte 2 incorrect");
    TEST_ASSERT(src == 0xC101, "Source pointer incorrect after fill column");
}

static void test_draw_command_room_transition_transparency(void) {
    GBState gb;
    gb_init(&gb);

    /* Fill VRAM target area with initial 0x55 */
    for (uint16_t i = 0x9800; i < 0x9820; i++) {
        gb_write(&gb, i, 0x55);
    }

    /* Row transfer with 0xEE transparent byte: [0x11, 0xEE, 0x22] */
    uint16_t src = 0xC100;
    gb_write(&gb, 0xC100, 0x11);
    gb_write(&gb, 0xC101, 0xEE);
    gb_write(&gb, 0xC102, 0x22);

    uint16_t dst = 0x9800;
    /* Command: row mode (bit 7 = 0), count = 3 */
    uint8_t cmd = 0x02;

    DrawCommandToVRAMDuringRoomTransition(&gb, &src, dst, cmd);

    TEST_ASSERT(gb_read(&gb, 0x9800) == 0x11, "Transition byte 0 not written");
    TEST_ASSERT(gb_read(&gb, 0x9801) == 0x55, "Transition byte 1 corrupted (should be skipped)");
    TEST_ASSERT(gb_read(&gb, 0x9802) == 0x22, "Transition byte 2 not written");
}

static void test_execute_draw_commands(void) {
    GBState gb;
    gb_init(&gb);

    /* Build a draw command sequence in WRAM at 0xD601:
     * Command 1: dest = 0x9800, mode = DC_FILL_ROW | 2 (fill 3 bytes with 0x44)
     * Command 2: dest = 0x9820, mode = DC_COPY_ROW | 1 (copy 2 bytes: 0x88, 0x99)
     * Terminator: 0x00
     */
    uint16_t cmd_ptr = 0xD601;

    /* Cmd 1 */
    gb_write(&gb, cmd_ptr++, 0x98); /* dest high */
    gb_write(&gb, cmd_ptr++, 0x00); /* dest low */
    gb_write(&gb, cmd_ptr++, DC_FILL_ROW | 0x02); /* length = 3, fill row */
    gb_write(&gb, cmd_ptr++, 0x44); /* fill byte */

    /* Cmd 2 */
    gb_write(&gb, cmd_ptr++, 0x98); /* dest high */
    gb_write(&gb, cmd_ptr++, 0x20); /* dest low */
    gb_write(&gb, cmd_ptr++, DC_COPY_ROW | 0x01); /* length = 2, copy row */
    gb_write(&gb, cmd_ptr++, 0x88);
    gb_write(&gb, cmd_ptr++, 0x99);

    /* End terminator */
    gb_write(&gb, cmd_ptr++, 0x00);

    /* Set wRoomTransitionState = 0 */
    gb_write(&gb, wRoomTransitionState, 0);

    ExecuteDrawCommands(&gb, 0xD601);

    /* Verify Cmd 1 */
    TEST_ASSERT(gb_read(&gb, 0x9800) == 0x44, "Cmd 1 byte 0 incorrect");
    TEST_ASSERT(gb_read(&gb, 0x9801) == 0x44, "Cmd 1 byte 1 incorrect");
    TEST_ASSERT(gb_read(&gb, 0x9802) == 0x44, "Cmd 1 byte 2 incorrect");

    /* Verify Cmd 2 */
    TEST_ASSERT(gb_read(&gb, 0x9820) == 0x88, "Cmd 2 byte 0 incorrect");
    TEST_ASSERT(gb_read(&gb, 0x9821) == 0x99, "Cmd 2 byte 1 incorrect");
}

static void test_no_room_transition_draw_loop(void) {
    GBState gb;
    gb_init(&gb);

    /* Command at 0xD610:
     * Pointing to destLow: 0x40
     * Mode: DC_FILL_ROW | 1 (fill 2 bytes with 0x5E)
     * Terminator: 0x00
     */
    uint16_t de = 0xD610;
    gb_write(&gb, de++, 0x40); /* dest low */
    gb_write(&gb, de++, DC_FILL_ROW | 0x01); /* length = 2, fill row */
    gb_write(&gb, de++, 0x5E); /* fill byte */
    gb_write(&gb, de++, 0x00); /* terminator high byte */

    /* Enter with a_dest_high = 0x99, de = 0xD610 */
    NoRoomTransitionDrawLoop(&gb, 0xD610, 0x99);

    TEST_ASSERT(gb_read(&gb, 0x9940) == 0x5E, "NoRoomTransition byte 0 incorrect");
    TEST_ASSERT(gb_read(&gb, 0x9941) == 0x5E, "NoRoomTransition byte 1 incorrect");
}

static void test_copy_to_bg_map_0(void) {
    GBState gb;
    gb_init(&gb);

    /* Prepare 360 bytes at 0xC200 */
    uint16_t src = 0xC200;
    for (int i = 0; i < 360; i++) {
        gb_write(&gb, 0xC200 + i, (uint8_t)(i + 1));
    }

    uint16_t hl = src;
    CopyToBGMap0(&gb, &hl);

    /* Verify hl advanced by 360 */
    TEST_ASSERT(hl == 0xC200 + 360, "hl not advanced by 360 bytes");

    /* Verify 18 rows of 20 bytes in vBGMap0 */
    int src_idx = 0;
    for (int row = 0; row < 18; row++) {
        uint16_t row_addr = vBGMap0 + row * 32;
        for (int col = 0; col < 20; col++) {
            uint8_t expected = (uint8_t)(src_idx + 1);
            TEST_ASSERT(gb_read(&gb, row_addr + col) == expected, "BGMap tile incorrect");
            src_idx++;
        }
        /* Verify remaining 12 bytes of the row were not overwritten (are 0) */
        for (int col = 20; col < 32; col++) {
            TEST_ASSERT(gb_read(&gb, row_addr + col) == 0, "BGMap row padding overwritten");
        }
    }
}

static void test_copy_data_to_vram_nodma(void) {
    GBState gb;
    gb_init(&gb);

    /* Allocate test rom */
    uint8_t rom[0x10000];
    memset(rom, 0, sizeof(rom));
    /* At 0x4000 (b = 0x40): fill 0x100 bytes */
    for (int i = 0; i < 0x100; i++) {
        rom[0x4000 + i] = (uint8_t)(i ^ 0xAA);
    }
    gb_attach_rom(&gb, rom, sizeof(rom));

    /* b = 0x40, c = 0x10 (dest = 0x8000 + 0x1000 = 0x9000), return_bank = 4 */
    CopyDataToVRAM_noDMA(&gb, 0x40, 0x10, 4);

    for (int i = 0; i < 0x100; i++) {
        TEST_ASSERT(gb_read(&gb, 0x9000 + i) == (uint8_t)(i ^ 0xAA), "VRAM byte incorrect");
    }
    TEST_ASSERT(gb.rom_bank == 4, "ROM bank not restored to 4");
}

static void test_copy_data_to_vram(void) {
    GBState gb;
    gb_init(&gb);

    uint8_t rom[0x20000];
    memset(rom, 0, sizeof(rom));
    /* In Bank 2 (offset 0x8000): at 0x4000 in address space */
    for (int i = 0; i < 0x100; i++) {
        rom[2 * 0x4000 + i] = (uint8_t)(i + 0x33);
    }
    gb_attach_rom(&gb, rom, sizeof(rom));

    /* 1. Test DMG mode (hIsGBC = 0) */
    gb_write(&gb, hIsGBC, 0);
    CopyDataToVRAM(&gb, 2, 0x40, 0x05, 5); /* dest: 0x8000 + 0x500 = 0x8500 */
    for (int i = 0; i < 0x100; i++) {
        TEST_ASSERT(gb_read(&gb, 0x8500 + i) == (uint8_t)(i + 0x33), "DMG VRAM byte incorrect");
    }
    TEST_ASSERT(gb.rom_bank == 5, "DMG ROM bank not restored to 5");

    /* 2. Test CGB mode (hIsGBC = 1) */
    gb_write(&gb, hIsGBC, 1);
    CopyDataToVRAM(&gb, 2, 0x40, 0x08, 6); /* dest: 0x8000 + 0x800 = 0x8800 */
    for (int i = 0; i < 0x100; i++) {
        TEST_ASSERT(gb_read(&gb, 0x8800 + i) == (uint8_t)(i + 0x33), "CGB VRAM byte incorrect");
    }
    TEST_ASSERT(gb.rom_bank == 6, "CGB ROM bank not restored to 6");
}

static void test_func_bb5(void) {
    GBState gb;
    gb_init(&gb);

    for (int i = 0; i < 0x168; i++) {
        gb_write(&gb, 0xC300 + i, (uint8_t)(i + 7));
    }

    func_BB5(&gb, 0xC300);

    for (int i = 0; i < 0x168; i++) {
        TEST_ASSERT(gb_read(&gb, wIsFileSelectionArrowShifted + i) == (uint8_t)(i + 7), "func_BB5 byte mismatch");
    }
}

static void test_copy_bg_map_from_bank(void) {
    GBState gb;
    gb_init(&gb);

    /* Allocate test ROM with 360 tile bytes + 360 attribute bytes in bank 4 */
    uint8_t rom[0x20000];
    memset(rom, 0, sizeof(rom));
    /* Bank 4 is offset 4 * 0x4000 = 0x10000 */
    for (int i = 0; i < 360; i++) {
        rom[0x10000 + i] = (uint8_t)(i + 1);          /* Tiles */
        rom[0x10000 + 360 + i] = (uint8_t)(i + 0x80);  /* Attributes */
    }
    gb_attach_rom(&gb, rom, sizeof(rom));

    /* 1. DMG mode, non-photo-album */
    gb_write(&gb, hIsGBC, 0);
    gb_write(&gb, wGameplayType, GAMEPLAY_WORLD);
    gb_write(&gb, hMultiPurposeF, 0x11);

    CopyBGMapFromBank(&gb, 4, 0x4000);

    /* Verify tiles in vBGMap0 */
    int idx = 0;
    for (int r = 0; r < 18; r++) {
        uint16_t row = vBGMap0 + r * 32;
        for (int c = 0; c < 20; c++) {
            TEST_ASSERT(gb_read(&gb, row + c) == (uint8_t)(idx + 1), "DMG tile mismatch");
            idx++;
        }
    }
    TEST_ASSERT(gb.rom_bank == 0x11, "DMG return bank mismatch");

    /* 2. GBC mode with GAMEPLAY_PHOTO_ALBUM */
    gb_init(&gb);
    gb_attach_rom(&gb, rom, sizeof(rom));
    gb_write(&gb, hIsGBC, 1);
    gb_write(&gb, wGameplayType, GAMEPLAY_PHOTO_ALBUM);
    gb_write(&gb, hMultiPurposeF, 0x22);

    CopyBGMapFromBank(&gb, 4, 0x4000);

    /* Verify attributes in VRAM bank 1 */
    gb.vram_bank = 1;
    idx = 0;
    for (int r = 0; r < 18; r++) {
        uint16_t row = vBGMap0 + r * 32;
        for (int c = 0; c < 20; c++) {
            TEST_ASSERT(gb_read(&gb, row + c) == (uint8_t)(idx + 0x80), "GBC attribute mismatch");
            idx++;
        }
    }

    /* Verify tiles in VRAM bank 0 */
    gb.vram_bank = 0;
    idx = 0;
    for (int r = 0; r < 18; r++) {
        uint16_t row = vBGMap0 + r * 32;
        for (int c = 0; c < 20; c++) {
            TEST_ASSERT(gb_read(&gb, row + c) == (uint8_t)(idx + 1), "GBC tile mismatch");
            idx++;
        }
    }

    /* Verify photo album copy to wIsFileSelectionArrowShifted */
    for (int i = 0; i < 360; i++) {
        TEST_ASSERT(gb_read(&gb, wIsFileSelectionArrowShifted + i) == (uint8_t)(i + 1), "Photo album tile buffer mismatch");
    }

    /* Verify return bank */
    TEST_ASSERT(gb.rom_bank == 0x22, "GBC return bank mismatch");
}

static void test_copy_siren_instrument_tiles(void) {
    GBState gb;
    gb_init(&gb);

    /* Bank $0C is offset 0x0C * 0x4000 = 0x30000 */
    uint8_t rom[0x40000];
    memset(rom, 0, sizeof(rom));
    for (int i = 0; i < 0x40; i++) {
        rom[0x30000 + 0xD00 + i] = (uint8_t)(i + 0x50);
    }
    gb_attach_rom(&gb, rom, sizeof(rom));

    /* Initial bank is 7 */
    gb.rom_bank = 7;

    /* Destination 0x8D00, source in bank 0x0C at 0x4D00 */
    CopySirenInstrumentTiles(&gb, 0x8D00, 0x4D00);

    /* Verify 64 bytes copied */
    for (int i = 0; i < 0x40; i++) {
        TEST_ASSERT(gb_read(&gb, 0x8D00 + i) == (uint8_t)(i + 0x50), "Siren tiles byte mismatch");
    }

    /* Verify bank restored to 1 */
    TEST_ASSERT(gb.rom_bank == 1, "ROM bank not restored to 1 after siren tiles copy");
}

static void test_func_bc5(void) {
    GBState gb;
    gb_init(&gb);

    uint8_t rom[0x20000];
    memset(rom, 0, sizeof(rom));
    for (int i = 0; i < 16; i++) {
        rom[0x18000 + 0x200 + i] = (uint8_t)(i + 0x90);
    }
    gb_attach_rom(&gb, rom, sizeof(rom));

    gb_write(&gb, w2_D16A, 0x06);
    gb.rom_bank = 2;

    func_BC5(&gb, 0xC500, 0x4200, 16);

    for (int i = 0; i < 16; i++) {
        TEST_ASSERT(gb_read(&gb, 0xC500 + i) == (uint8_t)(i + 0x90), "func_BC5 byte mismatch");
    }

    TEST_ASSERT(gb.rom_bank == 0x28, "func_BC5 did not restore ROM bank to 0x28");
}

static void test_copy_color_dungeon_symbols(void) {
    GBState gb;
    gb_init(&gb);

    uint8_t rom[0x100000];
    memset(rom, 0, sizeof(rom));
    for (int i = 0; i < 32; i++) {
        rom[0xD4000 + 0xF00 + i] = (uint8_t)(i + 0x66);
    }
    gb_attach_rom(&gb, rom, sizeof(rom));

    gb.rom_bank = 3;

    CopyColorDungeonSymbols(&gb, 0x14);

    for (int i = 0; i < 32; i++) {
        TEST_ASSERT(gb_read(&gb, wAnimatedScrollingTilesStorage + i) == (uint8_t)(i + 0x66),
                    "Color dungeon symbols byte mismatch");
    }

    TEST_ASSERT(gb.rom_bank == 0x14, "CopyColorDungeonSymbols did not restore stacked bank");
}

#define RUN_COPY_DATA_TEST(name, code) \
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

int run_copy_data_tests(void) {
    printf("[TEST] Copy Data\n");
    RUN_COPY_DATA_TEST("CopyData", test_copy_data_basic());
    RUN_COPY_DATA_TEST("CopyDataFromBank", test_copy_data_from_bank());
    RUN_COPY_DATA_TEST("CopyData_trampoline", test_copy_data_trampoline());
    RUN_COPY_DATA_TEST("DrawCommandToVRAM", {
        test_draw_command_copy_row_wrapping();
        test_draw_command_fill_row();
        test_draw_command_copy_column();
        test_draw_command_fill_column();
    });
    RUN_COPY_DATA_TEST("DrawCommandToVRAMDuringRoomTransition", test_draw_command_room_transition_transparency());
    RUN_COPY_DATA_TEST("ExecuteDrawCommands", test_execute_draw_commands());
    RUN_COPY_DATA_TEST("NoRoomTransitionDrawLoop", test_no_room_transition_draw_loop());
    RUN_COPY_DATA_TEST("CopyToBGMap0", test_copy_to_bg_map_0());
    RUN_COPY_DATA_TEST("CopyDataToVRAM_noDMA", test_copy_data_to_vram_nodma());
    RUN_COPY_DATA_TEST("CopyDataToVRAM", test_copy_data_to_vram());
    RUN_COPY_DATA_TEST("func_BB5", test_func_bb5());
    RUN_COPY_DATA_TEST("CopyBGMapFromBank", test_copy_bg_map_from_bank());
    RUN_COPY_DATA_TEST("CopySirenInstrumentTiles", test_copy_siren_instrument_tiles());
    RUN_COPY_DATA_TEST("func_BC5", test_func_bc5());
    RUN_COPY_DATA_TEST("CopyColorDungeonSymbols", test_copy_color_dungeon_symbols());

    if (failures == 0) {
        printf("[PASS] Copy Data\n\n");
    } else {
        printf("[FAIL] Copy Data\n\n");
    }
    return failures;
}
