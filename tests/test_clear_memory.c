#include <stdio.h>
#include <assert.h>
#include "gb.h"
#include "home/clear_memory.h"

static int failures = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
            failures++; \
        } \
    } while (0)

static void test_clear_bytes_basic(void) {
    GBState gb;
    gb_init(&gb);

    /* Fill WRAM area with 0x55 */
    for (uint16_t addr = 0xC000; addr < 0xC100; addr++) {
        gb_write(&gb, addr, 0x55);
    }
    gb_write(&gb, hIsGBC, 1);

    /* Clear 64 bytes starting at 0xC010 */
    ClearBytes(&gb, 0xC010, 64);

    /* Verify bytes 0xC000 - 0xC00F are still 0x55 */
    for (uint16_t addr = 0xC000; addr < 0xC100; addr++) {
        if (addr >= 0xC010 && addr < 0xC050) {
            TEST_ASSERT(gb_read(&gb, addr) == 0x00, "Cleared byte not zero");
        } else {
            TEST_ASSERT(gb_read(&gb, addr) == 0x55, "Uncleared byte corrupted");
        }
    }

    /* Verify hIsGBC is preserved */
    TEST_ASSERT(gb_read(&gb, hIsGBC) == 1, "hIsGBC was not preserved");
}

static void test_clear_bytes_preserves_hisgbc(void) {
    GBState gb;
    gb_init(&gb);

    /* Test with hIsGBC = 1 */
    gb_write(&gb, hIsGBC, 1);
    ClearBytes(&gb, 0xFF80, 0x7E);
    TEST_ASSERT(gb_read(&gb, hIsGBC) == 1, "hIsGBC=1 not preserved when clearing HRAM");

    /* Test with hIsGBC = 0 */
    gb_write(&gb, hIsGBC, 0);
    ClearBytes(&gb, 0xFF80, 0x7E);
    TEST_ASSERT(gb_read(&gb, hIsGBC) == 0, "hIsGBC=0 not preserved when clearing HRAM");
}

static void test_clear_wram_bytes(void) {
    GBState gb;
    gb_init(&gb);

    /* Fill entire WRAM with 0xEE */
    for (uint16_t addr = 0xC000; addr < 0xE000; addr++) {
        gb_write(&gb, addr, 0xEE);
    }

    /* Clear 0x1000 bytes starting at wram0Section (0xC000) */
    ClearWRAMBytes(&gb, 0x1000);

    /* Verify 0xC000 - 0xCFFF are 0x00 */
    for (uint16_t addr = 0xC000; addr < 0xD000; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x00, "WRAM byte was not cleared");
    }

    /* Verify 0xD000 - 0xDFFF remain 0xEE */
    for (uint16_t addr = 0xD000; addr < 0xE000; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0xEE, "Bytes outside cleared WRAM were altered");
    }
}

static void test_clear_hram_bytes_and_wram(void) {
    GBState gb;
    gb_init(&gb);

    /* Fill WRAM and HRAM with 0xCC */
    for (uint16_t addr = 0xC000; addr < 0xE000; addr++) {
        gb_write(&gb, addr, 0xCC);
    }
    for (uint16_t addr = 0xFF80; addr <= 0xFFFE; addr++) {
        gb_write(&gb, addr, 0xCC);
    }
    gb_write(&gb, hIsGBC, 1);

    /* Clear 0x20 bytes starting at hGameValuesSection (0xFF90) + WRAM to 0xDF00 */
    ClearHRAMBytesAndWRAM(&gb, 0x20);

    /* Verify HRAM 0xFF90 - 0xFFAF are 0x00 */
    for (uint16_t addr = 0xFF90; addr < 0xFFB0; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x00, "HRAM range byte not zero");
    }

    /* Verify HRAM 0xFFB0 - 0xFFFD remain 0xCC */
    for (uint16_t addr = 0xFFB0; addr < 0xFFFE; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0xCC, "HRAM outside cleared range altered");
    }

    /* Verify hIsGBC preserved */
    TEST_ASSERT(gb_read(&gb, hIsGBC) == 1, "hIsGBC was altered");

    /* Verify WRAM 0xC000 - 0xDEFF are 0x00 */
    for (uint16_t addr = 0xC000; addr < 0xDF00; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x00, "WRAM byte not zero");
    }

    /* Verify stack area 0xDF00 - 0xDFFF remains 0xCC */
    for (uint16_t addr = 0xDF00; addr < 0xE000; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0xCC, "Stack area was altered");
    }
}

static void test_clear_hram_and_wram(void) {
    GBState gb;
    gb_init(&gb);

    /* Fill WRAM and HRAM with 0xFF */
    for (uint16_t addr = 0xC000; addr < 0xE000; addr++) {
        gb_write(&gb, addr, 0xFF);
    }
    for (uint16_t addr = 0xFF80; addr <= 0xFFFE; addr++) {
        gb_write(&gb, addr, 0xFF);
    }
    gb_write(&gb, 0xFF85, 0x77);
    gb_write(&gb, hIsGBC, 1);

    ClearHRAMAndWRAM(&gb);

    /* 0xFF80-0xFF8F untouched */
    TEST_ASSERT(gb_read(&gb, 0xFF85) == 0x77, "HRAM below 0xFF90 was altered");

    /* 0xFF90 - 0xFFFC cleared */
    for (uint16_t addr = 0xFF90; addr < 0xFFFD; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x00, "Game HRAM variable was not cleared");
    }

    /* 0xFFFD (hIsComputingFrame) remains 0xFF */
    TEST_ASSERT(gb_read(&gb, 0xFFFD) == 0xFF, "hIsComputingFrame was unexpectedly cleared");

    /* 0xFFFE (hIsGBC) remains 1 */
    TEST_ASSERT(gb_read(&gb, hIsGBC) == 1, "hIsGBC was not preserved");

    /* 0xC000 - 0xDEFF cleared */
    for (uint16_t addr = 0xC000; addr < 0xDF00; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x00, "WRAM was not cleared");
    }

    /* 0xDF00 - 0xDFFF stack untouched */
    for (uint16_t addr = 0xDF00; addr < 0xE000; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0xFF, "Stack area was altered");
    }
}

static void test_clear_wram_and_lower_hram(void) {
    GBState gb;
    gb_init(&gb);

    /* Fill WRAM and HRAM with 0xBB */
    for (uint16_t addr = 0xC000; addr < 0xE000; addr++) {
        gb_write(&gb, addr, 0xBB);
    }
    for (uint16_t addr = 0xFF80; addr <= 0xFFFE; addr++) {
        gb_write(&gb, addr, 0xBB);
    }
    gb_write(&gb, hIsGBC, 1);

    ClearWRAMAndLowerHRAM(&gb);

    /* 0xFF90 - 0xFFBE (0x2F bytes) cleared */
    for (uint16_t addr = 0xFF90; addr < 0xFFBF; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x00, "Lower HRAM was not cleared");
    }

    /* 0xFFBF (hNextDefaultMusicTrack) and above remain 0xBB */
    for (uint16_t addr = 0xFFBF; addr < 0xFFFE; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0xBB, "Higher HRAM unexpectedly altered");
    }

    /* 0xFFFE (hIsGBC) remains 1 */
    TEST_ASSERT(gb_read(&gb, hIsGBC) == 1, "hIsGBC was not preserved");

    /* 0xC000 - 0xDEFF cleared */
    for (uint16_t addr = 0xC000; addr < 0xDF00; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x00, "WRAM was not cleared");
    }

    /* 0xDF00 - 0xDFFF stack untouched */
    for (uint16_t addr = 0xDF00; addr < 0xE000; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0xBB, "Stack area was altered");
    }
}

static void test_clear_lower_wram(void) {
    GBState gb;
    gb_init(&gb);

    /* Fill WRAM with 0xAA */
    for (uint16_t addr = 0xC000; addr < 0xE000; addr++) {
        gb_write(&gb, addr, 0xAA);
    }

    ClearLowerWRAM(&gb);

    /* 0xC000 - 0xD2FF cleared ($1300 bytes) */
    for (uint16_t addr = 0xC000; addr < 0xD300; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x00, "Lower WRAM was not cleared");
    }

    /* 0xD300 (wAudioSection) - 0xDFFF preserved */
    for (uint16_t addr = 0xD300; addr < 0xE000; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0xAA, "Audio section was altered");
    }
}

static void test_clear_lower_and_middle_wram(void) {
    GBState gb;
    gb_init(&gb);

    /* Fill WRAM with 0x99 */
    for (uint16_t addr = 0xC000; addr < 0xE000; addr++) {
        gb_write(&gb, addr, 0x99);
    }

    ClearLowerAndMiddleWRAM(&gb);

    /* 0xC000 - 0xD5FF cleared ($1600 bytes) */
    for (uint16_t addr = 0xC000; addr < 0xD600; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x00, "Lower/middle WRAM was not cleared");
    }

    /* 0xD600 (wDrawCommandsSection) - 0xDFFF preserved */
    for (uint16_t addr = 0xD600; addr < 0xE000; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x99, "Draw commands section was altered");
    }
}

static void test_fill_bg_map(void) {
    GBState gb;
    gb_init(&gb);

    /* Fill 64 bytes at vBGMap0 ($9800) with 0x42 */
    FillBGMap(&gb, 0x42, 64);

    for (uint16_t addr = 0x9800; addr < 0x9840; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x42, "Filled byte not correct");
    }
    TEST_ASSERT(gb_read(&gb, 0x9840) == 0x00, "Byte outside fill altered");
}

static void test_fill_bg_map_black(void) {
    GBState gb;
    gb_init(&gb);

    FillBGMapBlack(&gb);

    for (uint16_t addr = 0x9800; addr < 0x9C00; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == DIALOG_BG_TILE_DARK, "vBGMap0 not black tile");
    }
    TEST_ASSERT(gb_read(&gb, 0x9C00) == 0x00, "vBGMap1 unexpectedly modified by FillBGMapBlack");
}

static void test_fill_bg_map_white(void) {
    GBState gb;
    gb_init(&gb);

    FillBGMapWhite(&gb);

    for (uint16_t addr = 0x9800; addr < 0xA000; addr++) {
        TEST_ASSERT(gb_read(&gb, addr) == 0x7F, "vBGMap0/1 not white tile");
    }
    TEST_ASSERT(gb_read(&gb, 0xA000) == 0x00, "SRAM unexpectedly modified by FillBGMapWhite");
}

#define RUN_CLEAR_MEM_TEST(name, code) \
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

int run_clear_memory_tests(void) {
    printf("[TEST] Clear Memory\n");
    RUN_CLEAR_MEM_TEST("ClearBytes", {
        test_clear_bytes_basic();
        test_clear_bytes_preserves_hisgbc();
    });
    RUN_CLEAR_MEM_TEST("ClearWRAMBytes", test_clear_wram_bytes());
    RUN_CLEAR_MEM_TEST("ClearHRAMBytesAndWRAM", test_clear_hram_bytes_and_wram());
    RUN_CLEAR_MEM_TEST("ClearHRAMAndWRAM", test_clear_hram_and_wram());
    RUN_CLEAR_MEM_TEST("ClearWRAMAndLowerHRAM", test_clear_wram_and_lower_hram());
    RUN_CLEAR_MEM_TEST("ClearLowerWRAM", test_clear_lower_wram());
    RUN_CLEAR_MEM_TEST("ClearLowerAndMiddleWRAM", test_clear_lower_and_middle_wram());
    RUN_CLEAR_MEM_TEST("FillBGMap", test_fill_bg_map());
    RUN_CLEAR_MEM_TEST("FillBGMapBlack", test_fill_bg_map_black());
    RUN_CLEAR_MEM_TEST("FillBGMapWhite", test_fill_bg_map_white());

    if (failures == 0) {
        printf("[PASS] Clear Memory\n\n");
    } else {
        printf("[FAIL] Clear Memory\n\n");
    }
    return failures;
}
